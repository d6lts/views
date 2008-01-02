<?php
// $Id$
/**
 * @file view.inc
 * Provides the view object type and associated methods.
 */

/**
 * @defgroup views_objects Objects that represent a View or part of a view.
 * @{
 * These objects are the core of Views do the bulk of the direction and
 * storing of data. All database activity is in these objects.
 */

/**
 * An object to contain all of the data to generate a view, plus the member
 * functions to build the view query, execute the query and render the output.
 */
class view extends views_db_object {
  var $db_table = 'views_view';
  var $base_table = 'node';

  // State variables
  var $built = FALSE;
  var $executed = FALSE;

  var $args = array();
  var $build_info = array();

  // pager variables
  var $use_pager = FALSE;
  var $page_size = 25;
  var $pager_element = 0;
  var $offset = 0;
  var $current_page = 0;
  /**
   * Constructor
   */
  function view() {
    parent::init();
    // Make sure all of our sub objects are arrays.
    foreach ($this->objects_all() as $object) {
      $this->$object = array();
    }
  }

  /**
   * Returns a list of the sub-object types used by this view.
   */
  function objects() {
    return array('argument', 'field', 'sort', 'filter', 'relationship');
  }

  /**
   * Returns the complete list of dependent objects in a view, including
   * the display which is often special.
   *
   * @todo LG: special how?  This needs to be doc'ed more.
   * @todo Convert all references to this method to be static, not dynamic.
   */
  function objects_all() {
    return array('display', 'argument', 'field', 'sort', 'filter', 'relationship');
  }

  /**
   * Set the arguments that come to this view. Usually from the URL
   * but possibly from elsewhere.
   */
  function set_arguments($args) {
    $this->args = $args;
  }

  /**
   * Set the page size for ranged or pager queries
   *
   * @todo move all pager properties into a pager object
   */
  function set_page_size($page_size) {
    $this->page_size = $page_size;
  }

  /**
   * Whether or not the pager should be used.
   */
  function set_use_pager($use_pager) {
    $this->use_pager = $use_pager;
  }

  /**
   * The pager element id to use if use_apger is on
   */
  function set_pager_element($pager_element) {
    $this->pager_element = $pager_element;
  }

  /**
   * How many records to skip. This does not function if use_pager is
   * set.
   */
  function set_offset($offset) {
    $this->offset = $offset;
  }

  /**
   * Set the exposed filters input to an array. If unset they will be taken
   * from $_GET when the time comes.
   */
  function set_filter_input($filters) {
    $this->filter_input = $filters;
  }

  /**
   * Get an array of fields for the given display.
   */
  function get_fields($display_id = NULL) {
    if (!isset($display_id)) {
      $display_id = $this->current_display;
    }

    // @todo: Fix this:
    return $this->field;
  }

  /**
   * Set the display for this view, and initialize the display handler.
   *
   * @todo Determine if this is meant to be called only once, and if so, document that.
   */
  function set_display($display_id = NULL) {
    // The default display is always the first one in the list.
    if (isset($this->current_display)) {
      return TRUE;
    }

    $this->default_display = 0;
    if (!isset($display_id)) {
      $this->current_display = 0;
    }

    foreach ($this->display as $id => $display) {
      if ($display->id == $display_id) {
        $this->current_display = $id;
      }
    }

    // If the id was invalid, then just use the default display.
    if (!isset($this->current_display)) {
      $this->current_display = 0;
    }

    if (!isset($this->display[$this->current_display])) {
      return FALSE;
    }

    // Fetch the display handler data and instantiate an object.
    $display_data = $this->display[$this->current_display];
    $this->display_handler = views_get_plugin('display', $display_data->display_plugin);
    if (empty($this->display_handler)) {
      return FALSE;
    }

    // init the new display handler with data.
    $this->display_handler->init($this, $display_data);

    // If this is NOT the default display handler, let it know which is
    // since it may well utilize some data from the default.
    if ($this->current_display != $this->default_display) {
      $this->display_handler->default_display = $this->display[$this->default_display];
    }

    return TRUE;
  }

  /**
   * Find and initialize the style plugin.
   *
   * Note that arguments may have changed which style plugin we use, so
   * check the view object first, then ask the display handler.
   */
  function init_style() {
    if (!isset($this->style_plugin)) {
      $this->style_plugin = $this->display[$this->current_display]->style_plugin;
    }

    dsm("Init_style: $this->style_plugin");
    $this->style_handler = views_get_plugin('style', $this->style_plugin);
    if (empty($this->style_handler)) {
      return FALSE;
    }

    // init the new display handler with data.
    $this->style_handler->init($this, $this->display[$this->current_display]);
  }

  /**
   * Build the query for the view.
   */
  function build($display_id = NULL) {
    if (!empty($this->built)) {
      return;
    }

    if (!$this->set_display($display_id)) {
      return FALSE;
    }

    // Execute the initial PHP code that a view can have.
    if (!empty($this->view_args_php)) {
      ob_start();
      $result = eval($this->view_args_php);
      if (is_array($result)) {
        $this->args = $result;
      }
      ob_end_clean();
    }

    // Attempt to load from cache.
    // @todo: Load a build_info from cache.



    // If that fails, let's build!
    $this->build_info = array();

    $views_data = views_fetch_data($this->base_table);
    $this->base_field = $views_data['table']['base']['field'];
    views_include('query');
    $this->query = new views_query($this->base_table, $this->base_field);

    // Call a module hook and see if it wants to present us with a
    // pre-built query or instruct us not to build the query for
    // some reason.
    // @todo: Implement this.

    // Run through our handlers and ensure they have necessary information.
    $this->init_handlers();

    // Build all the filters.
    $this->_build('filter');

    // Initially, we want to build sorts and fields. This can change, though,
    // if we get a summary view.
    $this->build_sort = $this->build_fields = TRUE;

    $argument_title = '';
    // build arguments.
    foreach ($this->argument as $id => $arg) {
      unset ($argument);  // @todo: LG: Is this necessary, or is this because of the non-bug with reference reassignment?
      $argument = &$this->argument[$id];

      if (!is_object($argument->handler)) {
        // @todo: Set some kind of warning.
        continue;
      }

      if (isset($this->args[$id]) && $argument->handler->validate($this->args[$id])) {
        // handle argument that is present.
        // @todo: Do we want to put in argument placeholders here
        // So that we can try to cache queries with arguments too?
        $argument->handler->argument = $this->args[$id];
        $argument->handler->query();

        // Test to see if we should use this argument's title
        if (!empty($argument->title)) {
          $argument_title = $id;
        }
      }
      else {
        // determine default condition and handle.
        if (!$argument->handler->default_action()) {
          return;
        }
      }
    }

    // @todo: if $argument_title is set, get the argument and
    // set it in the build info.

    // Build our sort criteria if we were instructed to do so.
    if (!empty($this->build_sort)) {
      $this->_build('sort');
    }

    // @todo: Test display to see if it needs fields.
    if (!empty($this->build_fields)) {
      $this->_build('field');
    }

    $this->build_info['query'] = $this->query->query();
    $this->build_info['count_query'] = $this->query->query(TRUE);
    $this->build_info['query_args'] = $this->query->get_where_args();
    $this->built = TRUE;
    return TRUE;
  }

  /**
   * Internal method to build an individual set of handlers.
   */
  function _build($key) {
    $array = &$this->$key;
    foreach ($array as $id => $data) {
      // @todo: we should report an error here if this is not an object.
      if (is_object($array[$id]->handler)) {
        $array[$id]->handler->query();
      }
    }
  }

  /**
   * Acquire and attach all of the handlers.
   */
  function init_handlers() {
    if (empty($this->inited)) {
      foreach ($this->objects() as $key) {
        $this->_init_handler($key);
      }
      $this->inited = TRUE;
    }
  }

  /**
   * Attach all of the handlers for each type.
   *
   * @param $key
   *   One of 'argument', 'field', 'sort', 'filter', 'relationship'
   */
  function _init_handler($key) {
    foreach ($this->$key as $data) {
      $handler = views_get_handler($data->tablename, $data->field, $key);
      if (is_object($handler)) {
        $handler->init($this, $data);
        $data->handler = $handler;
      }
    }
  }

  /**
   * Execute the view's query.
   */
  function execute($display_id = NULL) {
    if (!empty($this->executed)) {
      return TRUE;
    }

    if (empty($this->built)) {
      if (!$this->build($display_id)) {
        return FALSE;
      }
    }

    $query = db_rewrite_sql($this->build_info['query'], $this->base_table, $this->base_field);
    $args = $this->build_info['query_args'];

    // @todo: Fill in execution defaults if they are not set.
    // use_pager, page_size, offset

    $items = array();
    if ($query) {
      if (!empty($this->use_pager)) {
        $count_query = db_rewrite_sql($this->build_info['count_query'], $this->base_table, $this->base_field);
        $this->result = pager_query($query, $view->page_size, $view->use_pager - 1, $count_query, $args);
        $this->total_rows = $GLOBALS['pager_total_items'][$this->use_pager - 1];
      }
      else if (!empty($this->page_size)) {
        $offset = $this->current_page * $this->page_size + $this->offset;
        $this->result = db_query_range($query, $args, $offset, $this->page_size);
      }
      else {
        $this->result = db_query($query, $args);
      }
    }

    $this->executed = TRUE;
  }

  /**
   * Render this view for display.
   */
  function render($display_id = NULL) {
    // Check for cached output.
    // @todo: Implement this

    $this->execute($display_id);

    // Check to see if the build failed.
    if (empty($this->result)) {
      return;
    }

    // Initialize the style plugin.
    $this->init_style();

    return $this->display_handler->render();
  }

  /**
   * To be called externally. Execute the given display, with
   * the given arguments.
   */
  function execute_display($display_id = NULL, $args = array()) {
    // Prepare the view with the information we have.
    $this->set_arguments($args);
    $this->set_display($display_id);

    // Execute the view
    if (isset($this->display_handler)) {
      return $this->display_handler->execute();
    }
  }

  /**
   * Called to get hook_menu information from the view and the
   * named display handler.
   */
  function execute_hook_menu($display_id = NULL) {
    // Prepare the view with the information we have.
    $this->set_display($display_id);  // Wasn't this just done already in the menu hook in views_get_page_views()?

    // Execute the view
    if (isset($this->display_handler)) {
      return $this->display_handler->execute_hook_menu();
    }
  }

  /**
   * Determine if the given user has access to the view. Note that
   * this sets the display handler if it hasn't been.
   */
  function access($account) {
    if (empty($this->display_handler) && !$this->set_display()) {
      return FALSE;
    }

    if (!$account) {
      $account = $GLOBALS['user'];
    }

    return $this->display_handler->access($account);
  }

  /**
   * Get the view's current title. This can change depending upon how it
   * was built.
   *
   * @todo: Implement this.
   */
  function get_title($context) { }

  /**
   * Get the URL for the current view.
   *
   * @todo: Implement this.
   */
  function get_url($args = NULL) { }

  /**
   * Is this view cacheable?
   */
  function is_cacheable() {
    return $this->is_cacheable;
  }

  /**
   * Load a view from the database based upon either vid or name.
   *
   * This is a static factory method that implements internal caching for
   * view objects.
   *
   * @param $arg
   *  The name of the view or its internal view id (vid)
   * @param $reset
   *  If TRUE, reset this entry in the load cache.
   * @return A view object or NULL if it was not available.
   */
  function &load($arg, $reset = FALSE) {
    static $cache = array();

    // We want a NULL value to return TRUE here, so we can't use isset() or empty().
    if (!array_key_exists($arg, $cache) || $reset) {
      $where = (is_numeric($arg) ? "vid =  %d" : "name = '%s'");
      $data = db_fetch_object(db_query("SELECT * FROM {views_view} WHERE $where", $arg));
      if (empty($data)) {
        $cache[$arg] = NULL;
      }
      else {
        $view =& new view();
        $view->load_row($data);
        // Load all of our subtables.
        foreach ($view->objects_all() as $key) {
          $view->_load_sub_row($key);
        }
        $view->loaded = TRUE;
        $cache[$arg] = $view;
      }
    }

    // If the view is not defined in the database, check the default_views hook.
    if (is_null($cache[$arg]) && !is_numeric($arg)) {
      $cache[$arg] = views_get_default_view($arg);
    }

    return $cache[$arg];
  }

  /**
   * Static factory method to load a list of views based upon a $where clause.
   *
   * Although this method could be implemented to simply iterate over views::load(),
   * that would be very slow.  Buiding the views externally from unified queries is
   * much faster.
   *
   * @todo FROM LG: We should document why this is passing in literal query fragments
   *  since it's bound to confuse someone else as much as it did/does me. :-)
   */
  function load_views($where, $join = '') {
    // @todo: Integrate this in with caching.
    $result = db_query("SELECT DISTINCT v.* FROM {views_view} v $join WHERE $where");
    $views = array();

    // Load all the views.
    while ($data = db_fetch_object($result)) {
      $view = new view;
      $view->load_row($data);
      $view->loaded = TRUE;
      $views[$view->vid] = $view;
    }

    // Stop if we didn't get any views.
    if (!$views) {
      return array();
    }

    $vids = implode(', ', array_keys($views));
    // Now load all the subtables:
    foreach (view::objects_all() as $key) {
      $object_name = "views_$key";
      $result = db_query("SELECT * FROM {{$object_name}} WHERE vid IN ($vids) ORDER BY vid, position");

      while ($data = db_fetch_object($result)) {
        $object = new $object_name(FALSE);
        $object->load_row($data);
        array_push($views[$object->vid]->$key, $object);
      }
    }
    return $views;
  }

  /**
   * Load one of our sub tables.
   */
  function _load_sub_row($key) {
    $object_name = "views_$key";
    $result = db_query("SELECT * FROM {$object_name} WHERE vid = %d ORDER BY position", $this->vid);

    while ($data = db_fetch_object($result)) {
      $object = new $object_name(FALSE);
      $object->load_row($data);
      array_push($this->$key, $object); // @todo: Why is this array_push instead of just []?
    }
  }

  /**
   * Save the view to the database. If the view does not already exist,
   * A vid will be assigned to the view and also returned from this function.
   */
  function save() {
    if (!empty($this->vid)) {
      // remove existing table entries
      foreach ($this->objects_all() as $key) {
        db_query("DELETE from {views_" . $key . "} WHERE vid = %d", $this->vid);
        $this->_load_sub_row($key);
      }
    }

    $this->save_row(!empty($this->vid) ? 'vid' : FALSE);

    // Save all of our subtables.
    foreach ($this->objects_all() as $key) {
      $this->_save_rows($key);
    }

    cache_clear_all('views_urls', 'cache_views');
    cache_clear_all(); // clear the page cache as well.
  }

  /**
   * Save a row to the database for the given key, which is one of the
   * keys from view::objects_all()
   */
  function _save_rows($key) {
    foreach ($this->$key as $position => $object) {
      $object->position = $position;
      $object->vid = $this->vid;
      $object->save_row();
    }
  }

  /**
   * Delete the view from the database.
   */
  function delete() {
    if (empty($this->vid)) {
      return;
    }

    db_query("DELETE FROM {views_view} WHERE vid = %d", $this->vid);
    // Delete from all of our subtables as well.
    foreach ($this->objects_all() as $key) {
      db_query("DELETE from {views_" . $key . "} WHERE vid = %d", $this->vid);
      $this->_load_sub_row($key);
    }

    cache_clear_all('views_query:' . $this->name, 'cache_views');
    cache_clear_all(); // In Drupal 5.0 and later this clears the page cache only.

  }

  /**
   * Export a view as PHP code.
   */
  function export() {
    $output = '';
    $output .= $this->export_row('view');
    foreach ($this->objects_all() as $key) {
      $output .= '$view->' . $key . ' = array()' . ";\n";
      foreach ($this->$key as $object) {
        $output .= $object->export_row($key, '  ');
        $output .= '$view->' . $key . '[] = $' . $key . ";\n";
      }
    }
    return $output;
  }
}

/**
 * Base class for views' database objects.
 */
class views_db_object {
  /**
   * Initialize this object, setting values from schema defaults.
   *
   * @param $init
   *   If an array, this is a set of values from db_fetch_object to
   *   load. Otherwse, if TRUE values will be filled in from schema
   *   defaults.
   */
  function init($init = TRUE) {
    if (is_array($init)) {
      return $this->load_row($init);
    }

    if (!$init) {
      return;
    }

    $schema = drupal_get_schema($this->db_table);
    // Go through our schema and build correlations.
    foreach ($schema['fields'] as $field => $info) {
      if ($info['type'] == 'serial') {
        $this->$field = NULL;
      }
      if (!isset($this->$field)) {
        if (!empty($info['serialize']) && isset($info['serialized default'])) {
          $this->$field = unserialize($info['serialized default']);
        }
        else if (isset($info['default'])) {
          $this->$field = $info['default'];
        }
        else {
          $this->$field = '';
        }
      }
    }
  }

  /**
   * Write the row to the database.
   *
   * @param $update
   *   If true this will be an UPDATE query. Otherwise it will be an INSERT.
   */
  function save_row($update = NULL) {
    $schema = drupal_get_schema($this->db_table);
    $fields = $defs = $values = $serials = array();

    // Go through our schema and build correlations.
    foreach ($schema['fields'] as $field => $info) {
      // special case -- skip serial types if we are updating.
      if ($info['type'] == 'serial' && $update) {
        continue;
      }
      $fields[] = $field;
      switch ($info['type']) {
        case 'serial':
          $defs[] = '%s';
          $this->$field = 'NULL';
          $serials[] = $field;
          break;
        case 'int':
          $defs[] = '%d';
          break;
        case 'float':
        case 'numeric':
          $defs[] = '%f';
          break;
        default:
          $defs[] = "'%s'";
      }
      if (empty($info['serialize'])) {
        $values[] = $this->$field;
      }
      else {
        $values[] = serialize($this->$field);
      }
    }
    $query = '';
    if (!$update) {
      $query = "INSERT INTO {$this->db_table} (`" . implode('`, `', $fields) . '`) VALUES (' . implode(', ', $defs) . ')';
    }
    else {
      $query = '';
      foreach ($fields as $id => $field) {
        if ($query) {
          $query .= ', ';
        }
        $query .= $field . ' = ' . $defs[$id];
      }
      $query = "UPDATE {$this->db_table} SET " . $query . " WHERE $update = " . $this->$update;
    }
    db_query($query, $values);

    if ($serials) {
      // get last insert ids and fill them in.
      foreach ($serials as $field) {
        $this->$field = db_last_insert_id($this->db_table, $field);
      }
    }
  }

  /**
   * Load the object with a row from the database.
   *
   * This method is separate from the constructor in order to give us
   * more flexibility in terms of how the view object is built in different
   * contexts.
   *
   * @param $data
   *   An object from db_fetch_object. It should contain all of the fields
   *   that are in the schema.
   */
  function load_row($data) {
    $schema = drupal_get_schema($this->db_table);

    // Go through our schema and build correlations.
    foreach ($schema['fields'] as $field => $info) {
      $this->$field = empty($info['serialize']) ? $data->$field : unserialize($data->$field);
    }
  }

  /**
   * Generate PHP code for a single field.
   */
  function _export_field($field) {
    if (is_array($this->$field) && empty($this->$field)) {
      return 'array()';
    }
    if (is_bool($this->$field)) {
      return $this->$field ? 'TRUE' : 'FALSE';
    }
    return var_export($this->$field, TRUE);
  }

  /**
   * Export a loaded row to PHP code.
   *
   * @todo LG: I don't understand what a "row" is in this context.
   * @param $identifier
   *   The variable to assign the PHP code for this object to.
   * @param $indent
   *   An optional indentation for prettifying nested code.
   */
  function export_row($identifier = NULL, $indent = '') {
    if (!$identifier) {
      $identifier = $this->db_table;
    }
    $schema = drupal_get_schema($this->db_table);

    $output = $indent . '$' . $identifier . ' = new ' . get_class($this) . ";\n";
    // Go through our schema and build correlations.
    foreach ($schema['fields'] as $field => $info) {
      if (!empty($info['no export'])) {
        continue;
      }
      if (!isset($this->$field)) {
        if (isset($info['default'])) {
          $this->$field = $info['default'];
        }
        else {
          $this->$field = '';
        }

        // serialized defaults must be set as serialized.
        if (isset($info['serialize'])) {
          $this->$field = unserialize($this->$field);
        }
      }
      $output .= $indent . '$' . $identifier . '->' . $field . ' = ' . $this->_export_field($field) . ";\n";
    }
    return $output;
  }
}

/**
 * An argument in a view.
 */
class views_argument extends views_db_object {
  var $db_table = 'views_argument';
  function views_argument($init = TRUE) {
    parent::init($init);
    $this->default_argument = 'ignore';
  }
}

/**
 * A field in a view.
 */
class views_field extends views_db_object {
  var $db_table = 'views_field';
  function views_field($init = TRUE) {
    parent::init($init);
  }
}

/**
 * A sort criterion in a view.
 */
class views_sort extends views_db_object {
  var $db_table = 'views_sort';
  function views_sort($init = TRUE) {
    parent::init($init);
  }
}

/**
 * A filter in a view.
 */
class views_filter extends views_db_object {
  var $db_table = 'views_filter';
  function views_filter($init = TRUE) {
    parent::init($init);
  }
}

/**
 * A display type in a view.
 */
class views_display extends views_db_object {
  var $db_table = 'views_display';
  function views_display($init = TRUE) {
    parent::init($init);
  }
}

/**
 * A relationship in a view.
 */
class views_relationship extends views_db_object {
  var $db_table = 'views_relationship';
  function views_relationship($init = TRUE) {
    parent::init($init);
  }
}

/**
 * @}
 */
