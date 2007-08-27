<?php
// $Id$
/** 
 * @file view.inc
 * Provides the view object type and associated methods.
 */

/**
 * Returns an array of objects in a view.
 */
function views_objects() {
  return array('argument', 'field', 'sort', 'filter', 'relationship');
}

/**
 * Returns the complete list of objects in a view, including the display which
 * is often special.
 */
function views_objects_all() {
  return array('display', 'argument', 'field', 'sort', 'filter', 'relationship');
}

/**
 * Get a view from the database or from default views.
 *
 * @param $name
 *   The name of the view.
 */
function views_get_view($name) {
  $view = new view;
  if ($view->load($name)) {
    return $view;
  }
  // TODO: check for default views in cache.
}

/**
 * An object to contain all of the data to generate a view, plus the member
 * functions to build the view query, execute the query and render the output.
 */
class view {
  var $vid = 0;
  var $name = '';
  var $description = '';
  var $display = array();
  var $argument = array();
  var $field = array();
  var $sort = array();
  var $filter = array();
  var $relationship = array();

  var $base_table = 'node';

  var $view_args_php = '';

  // State variables
  var $is_built = FALSE;
  
  var $args = array();

  var $query = NULL;
  var $count_query = NULL;

  function set_args($args) {
    $this->args = $args;
  }

  function set_page_size($page_size) { 
    $this->page_size = $page_size;
  }

  function set_use_pager($use_pager) {
    $this->use_pager = $use_pager;
  }

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

  function get_fields($display_id = NULL) {
    if (!isset($display_id)) {
      $display_id = $this->current_display;
    }

    // TODO: Fix this:
    return $this->field;
  }

  /**
   * Set the display for this view, and initialize the display handler.
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

    // Seed the new display handler with data.
    $this->display_handler->seed($this, $display_data);
    
    // If this is NOT the default display handler, let it know which is
    // since it may well utilize some data from the default.
    if ($this->current_display != $this->default_display) {
      $this->display_handler->default_display = $this->display[$this->default_display];
    }

    // Find and initialize the style plugin. Note that arguments may have changed
    // which style plugin we use, so check the view object first, then ask
    // the display handler.
    if (!isset($this->style_plugin)) {
      $this->style_plugin = $display_data->style_plugin;
    }

    $this->style_handler = views_get_plugin('style', $this->style_plugin);
    if (empty($this->style_handler)) {
      return FALSE;
    }

    // Seed the new display handler with data.
    $this->style_handler->seed($this, $display_data);

    return TRUE;
  }


  function build($display_id = NULL) { 
    if (!empty($this->built)) {
      return;
    }

    if (!$this->set_display($display_id)) {
      return FALSE;
    }

    // Execute the initial PHP code that a view can have.
    if ($this->view_args_php) {
      ob_start();
      $result = eval($this->view_args_php);
      if (is_array($result)) {
        $this->args = $result;
      }
      ob_end_clean();
    }
   
    // Attempt to load from cache.
    // TODO: Load a build_info from cache.



    // If that fails, let's build!
    $this->build_info = array();

    $views_data = views_fetch_data($this->base_table);
    $this->base_field = $views_data['table']['base']['field'];
    $this->query = new views_query($this->base_table, $this->base_field);

    // Call a module hook and see if it wants to present us with a
    // pre-built query or instruct us not to build the query for
    // some reason.
    // TODO: Implement this.  

    // Run through our handlers and ensure they have necessary information.
    $this->_seed_handlers();

    // Build all the filters.
    $this->_build('filter');

    $this->build_sort = $this->build_fields = TRUE;

    $argument_title = '';
    // build arguments.
    foreach ($this->argument as $id => $arg) {
      unset ($argument);
      $argument = &$this->argument[$id];

      if (!is_object($argument->handler)) {
        // TODO: Set some kind of warning.
        continue;
      }

      if (isset($this->args[$id])) {
        // handle argument that is present.
        // TODO: Do we want to put in argument placeholders here
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

    // Build our sort criteria if we were instructed to do so.
    if (!empty($this->build_sort)) {
      $this->_build('sort');
    }

    // TODO: Test display to see if it needs fields.
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
      // TODO: we should report an error here if this is not an object.
      if (is_object($array[$id]->handler)) {
        $array[$id]->handler->query();
      }
    }
  }

  /**
   * Acquire and attach all of the handlers.
   */
  function _seed_handlers() {
    if (empty($this->seeded)) {
      foreach (views_objects() as $key) {
        $this->_seed_handler($key);
      }
      $this->seeded = TRUE;
    }
  }

  /**
   * Attach all of the handlers for each type.
   * 
   * @param $key
   *   One of 'argument', 'field', 'sort', 'filter', 'relationship'
   */
  function _seed_handler($key) {
    foreach ($this->$key as $data) {
      $handler = views_get_handler($data->tablename, $data->field, $key);
      if (is_object($handler)) {
        $handler->seed($this, $data);
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

    // TODO: Fill in execution defaults if they are not set.
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
    // TODO: Implement this
    
    // Make sure the query has already executed.
    if (empty($this->executed)) {
      $this->execute($display_id);
    }

    // Check to see if the build failed.
    if (empty($this->result)) {
      return;
    }

    return $this->display_handler->render();
  }

  function get_title($context) { }
  function get_url($args = NULL) { }
  function is_cacheable() { }

  /**
   * Load a view from the database based upon either vid or name.
   */
  function load($arg) {    
    $where = (is_numeric($arg) ? "vid =  %d" : "name = '%s'");
    $data = db_fetch_object(db_query("SELECT * FROM {views_view} WHERE $where", $arg));
    if (empty($data)) {
      return FALSE;
    }

    _views_unpack_schema($this, 'views_view', $data);

    // Load all of our subtables.
    foreach (views_objects_all() as $key) {
      $this->_load_row($key);
    }
    $view->loaded = TRUE;
    return TRUE;
  }

  /**
   * Load one of our sub tables.
   */
  function _load_row($key) {
    $object_name = "views_$key";
    $table = $object_name;
    $result = db_query("SELECT * FROM {$table} WHERE vid = %d ORDER BY position", $this->vid);

    while ($data = db_fetch_object($result)) {
      $object = new $object_name;
      _views_unpack_schema($object, $table, $data);
      array_push($this->$key, $object);
    }

  }

  /**
   * Save the view to the database. If the view does not already exist, 
   * A vid will be assigned to the view and also returned from this function.
   */
  function save() {
    if (!empty($this->vid)) {
      // remove existing table entries
      foreach (views_objects_all() as $key) {
        db_query("DELETE from {views_" . $key . "} WHERE vid = %d", $this->vid);
        $this->_load_row($key);
      }
    }

    _views_save_query('views_view', $this, !empty($this->vid) ? 'vid' : FALSE);

    // Save all of our subtables.
    foreach (views_objects_all() as $key) {
      $this->_save_rows($key);
    }

    cache_clear_all('views_urls', 'cache_views');
    cache_clear_all(); // clear the page cache as well.
  }

  /**
   * Save a row to the database for the given key, which is one of the
   * keys from views_objects_all()
   */
  function _save_rows($key) {
    foreach ($this->$key as $position => $object) {
      $object->position = $position;
      $object->vid = $this->vid;
      _views_save_query("views_" . $key, $object);
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
    foreach (views_objects_all() as $key) {
      db_query("DELETE from {views_" . $key . "} WHERE vid = %d", $this->vid);
      $this->_load_row($key);
    }

    cache_clear_all('views_query:' . $this->name, 'cache_views');
    cache_clear_all(); // In Drupal 5.0 and later this clears the page cache only.
  
  }

  function export() {
    require_once drupal_get_path('module', 'views') . '/includes/export.inc';
    views_export_view($this);
  }
}

/**
 * An argument in a view.
 */
class views_argument {
  var $table = '';
  var $field = '';
  var $relationship = '';
  var $type = '';
  var $default_action = '';
  var $title = '';
  var $wildcard = '';
  var $wildcard_text = '';
  var $summary_format;
  var $options = array();
  var $position = 0;
}

/**
 * A field in a view.
 */
class views_field {
  var $table = '';
  var $field = '';
  var $label = '';
  var $position = 0;
  // Options contains things like: Sortable, default sort, column, etc.
  // Based upon the needs of the output type.
  var $options = array();
}

/**
 * A sort criterion in a view.
 */
class views_sort {
  var $table = '';
  var $field = '';
  var $order = '';
  var $options = array();
  var $position = 0;
}

/**
 * A filter in a view.
 */
class views_filter {
  var $table = '';
  var $field = '';
  var $group = '';
  var $operator = '';
  var $value = '';
  var $options = '';
  var $exposed = FALSE;
  var $exposed_settings = array();
  var $position = 0;
}

/**
 * A display type in a view.
 */
class views_display {
  var $type = '';
  var $output_type = '';
  var $access = '';
  var $title = '';
  var $header = '';
  var $header_format = '';
  var $header_hide_if_empty = '';
  var $footer = '';
  var $footer_format = '';
  var $footer_hide_if_empty = '';
  var $empty = '';
  var $empty_format = '';
  var $use_pager = '';
  var $page_size = '';
  var $url = '';
  var $display_options = array();
  var $output_options = array();
  var $position = 0;

  var $filters_type = ''; // 'table', 'list'
  var $filters_location = ''; // 'view', 'block'
}

/**
 * A relationship in a view.
 */
class views_relationship {
  var $relationship = '';
  var $link = '';
  var $position = 0;
}


/**
 * Build an insert/update query based upon schema info.
 */
function _views_save_query($table, &$object, $update = NULL) {
  $schema = drupal_get_schema($table);
  $fields = $defs = $values = $serials = array();

  // Go through our schema and build correlations.
  foreach ($schema['fields'] as $field => $info) {
    if (!isset($object->$field)) {
      if (isset($info['default'])) {
        $object->$field = $info['default'];
      }
      else {
        $object->$field = '';
      }
    }
    // special case -- skip serial types if we are updating.
    if ($info['type'] == 'serial' && $update) {
      continue;
    }
    $fields[] = $field;
    switch ($info['type']) {
      case 'serial':
        $defs[] = '%s';
        $object->$field = 'NULL';
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
      $values[] = $object->$field;
    }
    else {
      $values[] = serialize($object->$field);
    }
  }
  $query = '';
  if (!$update) {
    $query = "INSERT INTO {$table} (`" . implode('`, `', $fields) . '`) VALUES (' . implode(', ', $defs) . ')';
  }
  else {
    $query = '';
    foreach ($fields as $id => $field) {
      if ($query) {
        $query .= ', ';
      }
      $query .= $field . ' = ' . $defs[$id];
    }
    $query = "UPDATE {$table} SET " . $query . " WHERE $update = " . $object->$update;
  }
  db_query($query, $values);

  if ($serials) {
    // get last insert ids and fill them in.
    foreach ($serials as $field) {
      $object->$field = db_last_insert_id($table, $field);
    }
  }
}

function _views_unpack_schema(&$object, $table, $data) {
  $schema = drupal_get_schema($table);
  // Go through our schema and build correlations.

  foreach ($schema['fields'] as $field => $info) {
    $object->$field = empty($info['serialize']) ? $data->$field : unserialize($data->$field);
  }
}
