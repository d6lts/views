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
  var $pager = array(
    'use_pager' => FALSE,
    'items_per_page' => 10,
    'element' => 0,
    'offset' => 0,
    'current_page' => 0,
  );

  // Exposed widget input
  var $exposed_data = array();
  var $exposed_input = array();
  /**
   * Constructor
   */
  function view() {
    parent::init();
    // Make sure all of our sub objects are arrays.
    foreach ($this->db_objects() as $object) {
      $this->$object = array();
    }
  }

  /**
   * Returns a list of the sub-object types used by this view. These types are
   * stored on the display, and are used in the build process.
   */
  function display_objects() {
    return array('argument', 'field', 'sort', 'filter', 'relationship');
  }

  /**
   * Returns the complete list of dependent objects in a view, for the purpose
   * of initialization and loading/saving to/from the database.
   *
   * Note: In PHP5 this should be static, but PHP4 doesn't support static
   * methods.
   */
  function db_objects() {
    return array('display');
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
  function set_items_per_page($items_per_page) {
    $this->pager['items_per_page'] = $items_per_page;
  }

  /**
   * Whether or not the pager should be used.
   */
  function set_use_pager($use_pager) {
    $this->pager['use_pager'] = $use_pager;
  }

  /**
   * The pager element id to use if use_apger is on
   */
  function set_pager_element($pager_element) {
    $this->pager['element'] = $pager_element;
  }

  /**
   * How many records to skip. This does not function if use_pager is
   * set.
   */
  function set_offset($offset) {
    $this->pager['offset'] = $offset;
  }

  /**
   * Set the exposed filters input to an array. If unset they will be taken
   * from $_GET when the time comes.
   */
  function set_exposed_input($filters) {
    $this->exposed_input = $filters;
  }

  /**
   * Set the display for this view and initialize the display handler.
   */
  function init_display($test = NULL) {
    // The default display is always the first one in the list.
    if (isset($this->current_display)) {
      return TRUE;
    }

    // Instantiate all displays
    foreach (array_keys($this->display) as $id) {
      // Correct for shallow cloning
      // Often we'll have a cloned view so we don't mess up each other's
      // displays, but the clone is pretty shallow and doesn't necessarily
      // clone the displays. We can tell this by looking to see if a handler
      // has already been set; if it has, but $this->current_display is not
      // set, then something is dreadfully wrong.
      if (!empty($this->display[$id]->handler)) {
        $this->display[$id] = drupal_clone($this->display[$id]);
        unset($this->display[$id]->handler);
      }
      $this->display[$id]->handler = views_get_plugin('display', $this->display[$id]->display_plugin);
      if (!empty($this->display[$id]->handler)) {
        // Initialize the new display handler with data.
        $this->display[$id]->handler->init($this, $this->display[$id]);
        // If this is NOT the default display handler, let it know which is
        // since it may well utilize some data from the default.
        // This assumes that the 'default' handler is always first. It always
        // is. Make sure of it.
        if ($id != 'default') {
          $this->display[$id]->handler->default_display = &$this->display['default']->handler;
        }
      }
    }

    $this->current_display = 'default';
    $this->display_handler = &$this->display['default']->handler;

    return TRUE;
  }

  /**
   * Set the display as current.
   *
   * @param $display_id
   *   The id of the display to mark as current.
   */
  function set_display($display_id = NULL) {
    // If we have not already initialized the display, do so. But be careful.
    if (empty($this->current_display)) {
      $this->init_display();

      // If handlers were not initialized, and no argument was sent, set up
      // to the default display.
      if (empty($display_id)) {
        $display_id = 'default';
      }
    }

    // If no display id sent in and one wasn't chosen above, we're finished.
    if (empty($display_id)) {
      return TRUE;
    }

    // Ensure the requested display exists.
    if (empty($this->display[$display_id])) {
      $display_id = 'default';
      if (empty($this->display[$display_id])) {
        // @todo: Log an error here?
        return FALSE;
      }
    }

    // Set the current display.
    $this->current_display = $display_id;

    // Ensure requested display has a working handler.
    if (empty($this->display[$display_id]->handler)) {
      return FALSE;
    }

    // Set a shortcut
    $this->display_handler = &$this->display[$display_id]->handler;

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
      $this->style_plugin = $this->display_handler->get_option('style_plugin');
      $this->style_options = $this->display_handler->get_option('style_options');
    }

    vpr("Init_style: $this->style_plugin");
    $this->style_handler = views_get_plugin('style', $this->style_plugin);

    if (empty($this->style_handler)) {
      return FALSE;
    }

    // init the new display handler with data.
    $this->style_handler->init($this, $this->display[$this->current_display], $this->style_options);
  }

  /**
   * Acquire and attach all of the handlers.
   */
  function init_handlers() {
    if (empty($this->inited)) {
      foreach (views_object_types() as $key => $info) {
        $this->_init_handler($key, $info);
      }
      $this->inited = TRUE;
    }
  }

  /**
   * Run the pre_query() on all active handlers.
   */
  function _pre_query() {
    foreach (views_object_types() as $key => $info) {
      $item = &$this->$key;
      foreach ($item as $id => $info) {
        if (!empty($info['handler'])) {
          $item[$id]['handler']->pre_query();
        }
      }
    }
  }

  /**
   * Attach all of the handlers for each type.
   *
   * @param $key
   *   One of 'argument', 'field', 'sort', 'filter', 'relationship'
   */
  function _init_handler($key, $info) {
    // Load the requested items from the display onto the object.
    $this->$key = $this->display_handler->get_option($info['plural']);

    // Create a shortcut
    $items = &$this->$key;

    // Then run through and initialize a handler for each item.
    foreach ($items as $id => $data) {
      $handler = views_get_handler($data['table'], $data['field'], $key);
      if (is_object($handler)) {
        $handler->init($this, $data);
        // Deal with difficult PHP indirection:
        $items[$id]['handler'] = $handler;
      }
    }
  }

  /**
   * Build all the arguments.
   */
  function _build_arguments() {
    // Initially, we want to build sorts and fields. This can change, though,
    // if we get a summary view.
    if (empty($this->argument)) {
      return TRUE;
    }

    // build arguments.
    $position = -1;

    // Create a title for use in the breadcrumb trail.
    $title = $this->display_handler->get_option('title');

    $this->build_info['breadcrumb'] = array();
    $breadcrumb_args = array();
    $substitutions = array();

    // Iterate through each argument and process.
    foreach ($this->argument as $id => $arg) {
      $position++;
      $argument = &$this->argument[$id];

      if (!is_object($argument['handler'])) {
        // @todo: Set some kind of warning.
        continue;
      }

      if (isset($this->args[$position]) && $argument['handler']->validate($this->args[$position])) {
        // handle argument that is present.
        // @todo: Do we want to put in argument placeholders here
        // So that we can try to cache queries with arguments too?
        $argument['handler']->argument = $this->args[$position];
        $argument['handler']->query();

        // Since we're really generating the breadcrumb for the item above us,
        // check the default action of this argument.
        if ($this->display_handler->uses_breadcrumb() && $argument['handler']->uses_breadcrumb()) {
          $path = $this->get_url($breadcrumb_args);
          if (strpos('%', $path) === FALSE) {
            $this->build_info['breadcrumb'][$path] = str_replace(array_keys($substitutions), $substitutions, $title);
          }
        }

        // Allow the argument to muck with this breadcrumb.
        $argument['handler']->set_breadcrumb($this->build_info['breadcrumb']);

        // Test to see if we should use this argument's title
        if (!empty($argument['title'])) {
          $title = $argument['title'];
        }

        // Add this argument's substitution
        $substitutions['%' . ($position + 1)] = $argument['handler']->title();
        $breadcrumb_args[] = $this->args[$position];
      }
      else {
        // determine default condition and handle.
        return $argument['handler']->default_action();
      }
    }

    // set the title in the build info.
    if (!empty($title)) {
      $this->build_info['title'] = str_replace(array_keys($substitutions), $substitutions, $title);
    }

    return TRUE;
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
    // @todo Load a build_info from cache.

    // If that fails, let's build!
    $this->build_info = array();

    // Create and initialize the query object.
    $views_data = views_fetch_data($this->base_table);
    $this->base_field = $views_data['table']['base']['field'];
    if (!empty($views_data['table']['base']['database'])) {
      $this->base_database = $views_data['table']['base']['database'];
    }
    views_include('query');
    $this->query = new views_query($this->base_table, $this->base_field);

    // Call a module hook and see if it wants to present us with a
    // pre-built query or instruct us not to build the query for
    // some reason.
    // @todo: Implement this. Use the same mechanism Panels uses.

    // Run through our handlers and ensure they have necessary information.
    $this->init_handlers();

    // Let the handlers interact with each other if they really want.
    $this->_pre_query();

    if ($this->display_handler->uses_exposed()) {
      // Deal with any exposed filters we may have, before building.
      $this->exposed_widgets = views_get_form_get('views_exposed_form', $this, $this->display_handler->display);
    }

    // Build all the filters.
    $this->_build('filter');

    $this->build_sort = TRUE;

    // Arguments can, in fact, cause this whole thing to abort.
    if (!$this->_build_arguments()) {
      return;
    }

    // Build our sort criteria if we were instructed to do so.
    if (!empty($this->build_sort)) {
      $this->_build('sort');
    }

    // Initialize the style; arguments may have changed which style we use,
    // so waiting as long as possible is important. But we need to know
    // about the style when we go to build fields.
    $this->init_style();
    if ($this->style_handler->uses_fields()) {
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
      if (!empty($array[$id]['handler']) && is_object($array[$id]['handler'])) {
        // Give this handler access to the exposed filter input.
        if (!empty($this->exposed_data)) {
          if (!$array[$id]['handler']->accept_exposed_input($this->exposed_data)) {
            continue;
          }
        }
        $array[$id]['handler']->query();
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

    vpr($query);

    $items = array();
    if ($query) {
      // Allow for a view to query an external database.
      if (isset($this->base_database)) {
        db_set_active($this->base_database);
        $external = TRUE;
      }
      if (!empty($this->pager['use_pager'])) {
        $count_query = db_rewrite_sql($this->build_info['count_query'], $this->base_table, $this->base_field);

        $result = pager_query($query, $this->pager['items_per_page'], $this->pager['element'], $count_query, $args);
        $this->total_rows = $GLOBALS['pager_total_items'][$this->pager['element']];
      }
      else if (!empty($this->pager['items_per_page'])) {
        $offset = $this->pager['current_page'] * $this->pager['items_per_page'] + $this->pager['offset'];
        $result = db_query_range($query, $args, $offset, $this->pager['items_per_page']);

        // Rewrite and run the count query so we can store how many items this would have
        // matched.
        $count_query = db_rewrite_sql($this->build_info['count_query'], $this->base_table, $this->base_field);
        $this->total_rows = db_result(db_query($count_query, $args));
      }
      else {
        $result = db_query($query, $args);
      }

      $this->result = array();
      while ($item = db_fetch_object($result)) {
        $this->result[] = $item;
      }
      if ($external) {
        db_set_active();
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
    if (!isset($this->result)) {
      return;
    }

    // Initialize the style plugin.
    $this->init_style();

    // Give field handlers the opportunity to perform additional queries
    // using the entire resultset prior to rendering.
    foreach ($this->field as $id => $field) {
      $this->field[$id]['handler']->pre_render($this->result);
    }

    return $this->display_handler->render();
  }

  /**
   * To be called externally. Execute the given display, with
   * the given arguments.
   */
  function execute_display($display_id = NULL, $args = array()) {
    if (empty($this->current_display) || $this->current_display != $display_id) {
      if (!$this->set_display($display_id)) {
        return FALSE;
      }
    }

    // Prepare the view with the information we have.
    $this->set_arguments($args);

    // Copy pager information from the display.
    // @todo: Should this code live on the display?
    $this->set_use_pager($this->display_handler->use_pager());
    $this->set_pager_element($this->display_handler->get_option('pager_element'));
    $this->set_items_per_page($this->display_handler->get_option('items_per_page'));
    $this->set_offset($this->display_handler->get_option('offset'));

    // Execute the view
    return $this->display_handler->execute();
  }

  /**
   * Called to get hook_menu information from the view and the
   * named display handler.
   */
  function execute_hook_menu($display_id = NULL) {
    // Prepare the view with the information we have.

    // This was probably already called, but it's good to be safe.
    if (!$this->set_display($display_id)) {
      return FALSE;
    }

    // Execute the view
    if (isset($this->display_handler)) {
      return $this->display_handler->execute_hook_menu();
    }
  }

  /**
   * Called to get hook_block information from the view and the
   * named display handler.
   */
  function execute_hook_block($display_id = NULL) {
    // Prepare the view with the information we have.

    // This was probably already called, but it's good to be safe.
    if (!$this->set_display($display_id)) {
      return FALSE;
    }

    // Execute the view
    if (isset($this->display_handler)) {
      return $this->display_handler->execute_hook_block();
    }
  }

  /**
   * Determine if the given user has access to the view. Note that
   * this sets the display handler if it hasn't been.
   */
  function access($display_id = NULL, $account = NULL) {
    if (!isset($this->current_display)) {
      return TRUE;
    }

    if (!$account) {
      $account = $GLOBALS['user'];
    }

    if (!empty($this->display[$display_id]->handler)) {
      return $this->display[$display_id]->handler->access($account);
    }
  }

  /**
   * Get the view's current title. This can change depending upon how it
   * was built.
   */
  function get_title() {
    if (empty($this->display_handler)) {
      if (!$this->set_display('default')) {
        return FALSE;
      }
    }

    // During building, we might find a title override. If so, use it.
    if (!empty($this->build_info['title'])) {
      $title = $this->build_info['title'];
    }
    else {
      $title = $this->display_handler->get_option('title');
    }

    return $title;
  }

  /**
   * Get the URL for the current view.
   *
   * This URL will be adjusted for arguments.
   */
  function get_url($args = NULL) {
    $path = $this->get_path();
    if (!isset($args)) {
      $args = $this->args;
    }
    // Don't bother working if there's nothing to do:
    if (empty($args) || empty($path)) {
      return $path;
    }

    $pieces = array();
    foreach (explode('/', $path) as $piece) {
      if ($piece != '%') {
        $pieces[] = $piece;
      }
      else {
        $pieces[] = empty($args) ? $piece : array_shift($args);
      }
    }

    return implode('/', array_merge($pieces, $args));
  }

  /**
   * Get the base path used for this view.
   */
  function get_path() {
    if (empty($this->display_handler)) {
      if (!$this->set_display('default')) {
        return FALSE;
      }
    }
    return $this->display_handler->get_path();
  }

  /**
   * Get the breadcrumb used for this view.
   *
   * @param $set
   *   If true, use drupal_set_breadcrumb() to install the breadcrumb.
   */
  function get_breadcrumb($set = FALSE) {
    // Now that we've built the view, extract the breadcrumb.
    $base = TRUE;
    $breadcrumb = array();

    if (!empty($this->build_info['breadcrumb'])) {
      foreach ($this->build_info['breadcrumb'] as $path => $title) {
        // Check to see if the frontpage is in the breadcrumb trail; if it
        // is, we'll remove that from the actual breadcrumb later.
        if ($path == variable_get('site_frontpage', 'node')) {
          $base = FALSE;
          $title = t('Home');
        }
        $breadcrumb[] = l($title, $path, array('html' => true));
      }

      if ($set) {
        if ($base) {
          $breadcrumb = array_merge(drupal_get_breadcrumb(), $breadcrumb);
        }
        drupal_set_breadcrumb($breadcrumb);
      }
    }
    return $breadcrumb;
  }

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
        $view->type = t('Normal');
        // Load all of our subtables.
        foreach ($view->db_objects() as $key) {
          $object_name = "views_$key";
          $result = db_query("SELECT * FROM {{$object_name}} WHERE vid = %d ORDER BY position", $view->vid);

          while ($data = db_fetch_object($result)) {
            $object = new $object_name(FALSE);
            $object->load_row($data);

            // Because it can get complicated with this much indirection,
            // make a shortcut reference.
            $location = &$view->$key;

            // If we have a basic id field, load the item onto the view based on
            // this ID, otherwise push it on.
            if (!empty($object->id)) {
              $location[$object->id] = $object;
            }
            else {
              $location[] = $object;
            }
          }
        }

        $view->loaded = TRUE;
        $cache[$arg] = $view;
      }
    }

    return $cache[$arg];
  }

  /**
   * Static factory method to load a list of views based upon a $where clause.
   *
   * Although this method could be implemented to simply iterate over views::load(),
   * that would be very slow.  Buiding the views externally from unified queries is
   * much faster.
   */
  function load_views() {
    // @todo: Integrate this in with caching.
    $result = db_query("SELECT DISTINCT v.* FROM {views_view} v");
    $views = array();

    // Load all the views.
    while ($data = db_fetch_object($result)) {
      $view = new view;
      $view->load_row($data);
      $view->loaded = TRUE;
      $view->type = t('Normal');
      $views[$view->vid] = $view;
    }

    // Stop if we didn't get any views.
    if (!$views) {
      return array();
    }

    $vids = implode(', ', array_keys($views));
    // Now load all the subtables:
    foreach (view::db_objects() as $key) {
      $object_name = "views_$key";
      $result = db_query("SELECT * FROM {{$object_name}} WHERE vid IN ($vids) ORDER BY vid, position");

      while ($data = db_fetch_object($result)) {
        $object = new $object_name(FALSE);
        $object->load_row($data);

        // Because it can get complicated with this much indirection,
        // make a shortcut reference.
        $location = &$views[$object->vid]->$key;

        // If we have a basic id field, load the item onto the view based on
        // this ID, otherwise push it on.
        if (!empty($object->id)) {
          $location[$object->id] = $object;
        }
        else {
          $location[] = $object;
        }
      }
    }
    return $views;
  }

  /**
   * Save the view to the database. If the view does not already exist,
   * A vid will be assigned to the view and also returned from this function.
   */
  function save() {
    if ($this->vid == 'new') {
      $this->vid = NULL;
    }

    // If we have no vid or our vid is a string, this is a new view.
    if (!empty($this->vid)) {
      // remove existing table entries
      foreach ($this->db_objects() as $key) {
        db_query("DELETE from {views_" . $key . "} WHERE vid = %d", $this->vid);
      }
    }

    $this->save_row(!empty($this->vid) ? 'vid' : FALSE);

    // Save all of our subtables.
    foreach ($this->db_objects() as $key) {
      $this->_save_rows($key);
    }

    cache_clear_all('views_urls', 'cache_views');
    cache_clear_all(); // clear the page cache as well.
  }

  /**
   * Save a row to the database for the given key, which is one of the
   * keys from view::db_objects()
   */
  function _save_rows($key) {
    $count = 0;
    foreach ($this->$key as $position => $object) {
      $object->position = ++$count;
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
    foreach ($this->db_objects() as $key) {
      db_query("DELETE from {views_" . $key . "} WHERE vid = %d", $this->vid);
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
    // Set the API version
    $output .= '$view->api_version = 2' . ";\n";
    $output .= '$view->disabled = FALSE; // Edit this to true to make a default view disabled initially' . "\n";

    foreach ($this->db_objects() as $key) {
      $output .= '$view->' . $key . ' = array()' . ";\n";
      foreach ($this->$key as $id => $object) {
        $output .= $object->export_row($key, '  ');
        $output .= '$view->' . $key . '[\'' . $id . '\'] = $' . $key . ";\n";
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
      $query = "INSERT INTO {" . $this->db_table . "} (" . implode(', ', $fields) . ') VALUES (' . implode(', ', $defs) . ')';
    }
    else {
      $query = '';
      foreach ($fields as $id => $field) {
        if ($query) {
          $query .= ', ';
        }
        $query .= $field . ' = ' . $defs[$id];
      }
      $query = "UPDATE {" . $this->db_table . "} SET " . $query . " WHERE $update = " . $this->$update;
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
   * Export a loaded row, such as an argument, field or the view itself to PHP code.
   *
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

  /**
   * Add a new display handler to the view, automatically creating an id.
   *
   * @param $type
   *   The plugin type from the views plugin data. Defaults to 'page'.
   *
   * @return
   *   The key to the display in $view->display, so that the new display
   *   can be easily located.
   */
  function add_display($type = 'page') {
    if (empty($type)) {
      return FALSE;
    }

    $plugin = views_fetch_plugin_data('display', $type);
    if (empty($plugin)) {
      return FALSE;
    }

    $id = $type;
    $title = $plugin['title'];

    $count = 0;

    // Loop through IDs based upon our style plugin name until
    // we find one that is unused.
    while (!empty($this->display[$id])) {
      $id = $type . '_' . ++$count;
      $title = $plugin['title'] . ' ' . $count;
    }

    // Create the new display object
    $display = new views_display;
    $display->options($type, $id, $title);

    // Add the new display object to the view.
    $this->display[$id] = $display;
    return $id;
  }

  /**
   * Add an item with a handler to the view.
   *
   * These items may be fields, filters, sort criteria, or arguments.
   */
  function add_item($display_id, $type, $table, $field) {
    $types = views_object_types();
    $this->set_display($display_id);

    $fields = $this->display[$display_id]->handler->get_option($types[$type]['plural']);

    $count = 0;
    $id = $field;
    while (!empty($fields[$id])) {
      $id = $field . '_' . ++$count;
    }
    $new_item = array(
      'id' => $id,
      'table' => $table,
      'field' => $field,
    );

    $handler = views_get_handler($table, $field, $type);
    $handler->options($new_item);

    $fields[$id] = $new_item;
    $this->display[$display_id]->handler->set_option($types[$type]['plural'], $fields);

    return $id;
  }

  /**
   * Get an array of items for the current display.
   */
  function get_items($type, $display_id = NULL) {
    $this->set_display($display_id);

    if (!isset($display_id)) {
      $display_id = $this->current_display;
    }

    // Get info about the types so we can get the right data.
    $types = views_object_types();
    return $this->display[$display_id]->handler->get_option($types[$type]['plural']);
  }

  /**
   * Get the configuration of an item (field/sort/filter/etc) on a given
   * display.
   */
  function get_item($display_id, $type, $id) {
    // Get info about the types so we can get the right data.
    $types = views_object_types();
    // Initialize the display
    $this->set_display($display_id);

    // Get the existing configuration
    $fields = $this->display[$display_id]->handler->get_option($types[$type]['plural']);

    return isset($fields[$id]) ? $fields[$id] : NULL;
  }

  /**
   * Get the configuration of an item (field/sort/filter/etc) on a given
   * display.
   *
   * Pass in NULL for the $item to remove an item.
   */
  function set_item($display_id, $type, $id, $item) {
    // Get info about the types so we can get the right data.
    $types = views_object_types();
    // Initialize the display
    $this->set_display($display_id);

    // Get the existing configuration
    $fields = $this->display[$display_id]->handler->get_option($types[$type]['plural']);
    if (isset($item)) {
      $fields[$id] = $item;
    }
    else {
      unset($fields[$id]);
    }

    // Store.
    $this->display[$display_id]->handler->set_option($types[$type]['plural'], $fields);
  }
}

class views_sub_object {
  var $table = '';
  var $field = '';
  var $relationship = '';
}

/**
 * An argument in a view.
 */
class views_argument extends views_sub_object {
}

/**
 * A field in a view.
 */
class views_field  extends views_sub_object  {
}

/**
 * A sort criterion in a view.
 */
class views_sort  extends views_sub_object {
}

/**
 * A filter in a view.
 */
class views_filter  extends views_sub_object {
  var $group = 0;
}

/**
 * A relationship in a view.
 */
class views_relationship  extends views_sub_object {
}

/**
 * A display type in a view.
 */
class views_display extends views_db_object {
  var $db_table = 'views_display';
  function views_display($init = TRUE) {
    parent::init($init);
  }

  function options($type, $id, $title) {
    $this->display_plugin = $type;
    $this->id = $id;
    $this->display_title = $title;

    // Create a handler to fill in default values.
    $handler = views_get_plugin('display', $type);
    $handler->options($this);
  }
}


/**
 * @}
 */
