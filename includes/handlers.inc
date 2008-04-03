<?php
// $Id$
/**
 * @file handlers.inc
 * Defines the various handler objects to help build and display views.
 */

/**
 * Instantiate and construct a new handler
 */
function _views_create_handler($definition) {
//  vpr('Instantiating handler ' . $definition['handler']);
  $handler = new $definition['handler'];
  $handler->set_definition($definition);
  // let the handler have something like a constructor.
  if (isset($definition['arguments'])) {
    call_user_func_array(array(&$handler, 'construct'), $definition['arguments']);
  }
  else {
    $handler->construct();
  }

  return $handler;
}

/**
 * Prepare a handler's data by checking defaults and such.
 */
function _views_prepare_handler($definition, $data, $field) {
  foreach (array('group', 'title', 'help') as $key) {
    // First check the field level
    if (!isset($definition[$key]) && !empty($data[$field][$key])) {
      $definition[$key] = $data[$field][$key];
    }
    // Then if that doesn't work, check the table level
    if (!isset($definition['table'][$key]) && !empty($data['table'][$key])) {
      $definition[$key] = $data['table'][$key];
    }
  }

  return _views_create_handler($definition);
}

/**
 * Fetch a handler to join one table to a primary table from the data cache
 */
function views_get_table_join($table, $primary_table) {
  $data = views_fetch_data($table);
  if (isset($data['table']['join'][$primary_table])) {
    $h = $data['table']['join'][$primary_table];
    if (!empty($h['handler']) && class_exists($h['handler'])) {
      $handler = new $h['handler'];
    }
    else {
      $handler = new views_join();
    }

    // Fill in some easy defaults
    $handler->definition = $h;
    if (empty($handler->definition['table'])) {
      $handler->definition['table'] = $table;
    }
    // If this is empty, it's a direct link.
    if (empty($handler->definition['left_table'])) {
      $handler->definition['left_table'] = $primary_table;
    }

    if (isset($h['arguments'])) {
      call_user_func_array(array(&$handler, 'construct'), $h['arguments']);
    }
    else {
      $handler->construct();
    }

    return $handler;
  }
  // DEBUG -- identify missing handlers
  vpr("Missing join: $table $primary_table");
}

/**
 * Base handler, from which all the other handlers are derived.
 * It creates a common interface to create consistency amongst
 * handlers and data.
 *
 * The default handler has no constructor, so there's no need to jank with
 * parent::views_handler() here.
 *
 * This class would be abstract in PHP5, but PHP4 doesn't understand that.
 *
 */
class views_handler extends views_object {
  /**
   * A constructor for the handler base object
   *
   * This should be overridden to provide for a consistent constructor
   * mechanism.
   */
  function construct() { }

  /**
   * init the handler with necessary data.
   * @param $view
   *   The $view object this handler is attached to.
   * @param $options
   *   The item from the database; the actual contents of this will vary
   *   based upon the type of handler.
   */
  function init(&$view, $options) {
    $this->view = &$view;
    $this->options = &$options;

    // This exist on most handlers, but not all. So they are still optional.
    if (isset($options['table'])) {
      $this->table = $options['table'];
    }

    if (isset($options['field'])) {
      $this->field = $options['field'];
      if (!isset($this->real_field)) {
        $this->real_field = $options['field'];
      }
    }

    if (!empty($view->query)) {
      $this->query = &$view->query;
    }
  }

  /**
   * Return a string representing this handler's name in the UI.
   */
  function ui_name() {
    return t('@group: @title', array('@group' => $this->definition['group'], '@title' => $this->definition['title']));
  }

  /**
   * Provide defaults for the handler.
   */
  function options(&$option) { }

  /**
   * Provide a form for setting options.
   */
  function options_form(&$form, &$form_state) { }

  /**
   * Validate the options form.
   */
  function options_validate($form, &$form_state) { }

  /**
   * Perform any necessary changes to the form values prior to storage.
   * There is no need for this function to actually store the data.
   */
  function options_submit($form, &$form_state) { }

  /**
   * If a handler has 'extra options' it will get a little settings widget and
   * another form called extra_options.
   */
  function has_extra_options() { return FALSE; }

  /**
   * Provide defaults for the handler.
   */
  function extra_options(&$option) { }

  /**
   * Provide a form for setting options.
   */
  function extra_options_form(&$form, &$form_state) { }

  /**
   * Validate the options form.
   */
  function extra_options_validate($form, &$form_state) { }

  /**
   * Perform any necessary changes to the form values prior to storage.
   * There is no need for this function to actually store the data.
   */
  function extra_options_submit($form, &$form_state) { }

  /**
   * Set new exposed option defaults when exposed setting is flipped
   * on.
   */
  function expose_options() { }
  /**
   * Render our chunk of the exposed filter form when selecting
   */
  function exposed_form(&$form, &$form_state) { }

  /**
   * Validate the exposed filter form
   */
  function exposed_validate(&$form, &$form_state) { }

  /**
   * Submit the exposed filter form
   */
  function exposed_submit(&$form, &$form_state) { }

  /**
   * Get information about the exposed form for the form renderer.
   *
   * @return
   *   An array with the following keys:
   *   - operator: The $form key of the operator. Set to NULL if no operator.
   *   - value: The $form key of the value. Set to NULL if no value.
   *   - label: The label to use for this piece.
   */
  function exposed_info() { }

 /**
  * Check whether current user has access to this handler.
  *
  * @return boolean
  */
  function access() {
    return TRUE;
  }

  /**
   * Run before the view is built.
   *
   * This gives all the handlers some time to set up before any handler has
   * been fully run.
   */
  function pre_query() { }

  /**
   * Called just prior to query(), this lets a handler set up any relationship
   * it needs.
   */
  function set_relationship() {
    // Ensure this gets set to something.
    $this->relationship = NULL;

    // Don't process non-existant relationships.
    if (empty($this->options['relationship']) || $this->options['relationship'] == 'none') {
      return;
    }

    $relationship = $this->options['relationship'];

    // Ignore missing/broken relationships.
    if (empty($this->view->relationship[$relationship]) || empty($this->view->relationship[$relationship]['handler'])) {
      return;
    }

    // Check to see if the relationship has already processed. If not, then we
    // cannot process it.
    if (empty($this->view->relationship[$relationship]['handler']->alias)) {
      return;
    }

    // Finally!
    $this->relationship = $this->view->relationship[$relationship]['handler']->alias;
  }

  /**
   * Add this handler into the query.
   *
   * If we were using PHP5, this would be abstract.
   */
  function query() { }

  /**
   * Ensure the main table for this handler is in the query. This is used
   * a lot.
   */
  function ensure_my_table() {
    if (!isset($this->table_alias)) {
      $this->table_alias = $this->query->ensure_table($this->table, $this->relationship);
    }
    return $this->table_alias;
  }

  /**
   * Provide text for the administrative summary
   */
  function admin_summary() { }

  /**
   * Determine if the argument needs a style plugin.
   *
   * @return TRUE/FALSE
   */
  function needs_style_plugin() { return FALSE; }

  /**
   * Determine if this item is 'exposed', meaning it provides form elements
   * to let users modify the view.
   *
   * @return TRUE/FALSE
   */
  function is_exposed() {
    return !empty($this->options['exposed']);
  }

  /**
   * Take input from exposed filters and assign to this handler, if necessary.
   */
  function accept_exposed_input($input) { return TRUE; }
}


/**
 * @defgroup views_hierarchy Hierarchy helper functions
 * @{
 *
 * These functions help deal with the complexities of filtering
 * and joining hiearchy. Their use requires some extra preparation:
 *
 * @todo document that preparation.
 */

class views_term_filter {
  var $operator = NULL; // and, or, not
  var $value = array(); // An array of values to filter.
  var $depth = 0; // Maybe positive or negative.

  /**
   * This must be a views_join descended object that describes how the
   * base table joins onto the hierarchy. If there isn't a hierarchy
   * join, then this must be a join that describes how the base table
   * joins onto itself.
   */
  var $base_join = NULL;

  /**
   * The name of the base table.
   */
  var $base_table = NULL;

  /**
   * The name of the field we are filtering against.
   */
  var $field = NULL;

  /**
   * This must be a views_join descended object that describes how the
   * hierarchy table joins to the next hierarchy table.
   */
  var $hierarchy_join = NULL; // This must be set before this can be used.

  /**
   * The name of the hierarchy table.
   */
  var $hierarchy_table = NULL;

  /**
   * The relationship that starts this entire show.
   */
  var $base_relationship = NULL;

  var $handler = NULL; // So we can access information from the handler.

  /**
   * This will be a 2d array, referenced by chain and then depth.
   */
  var $base_tables = array();
  var $hierarchy_tables = array();

  /**
   * What placeholder to use in the query. "'%s'" by default; use %d if we
   * are doing string compares.
   */
  var $placeholder = "'%s'";
  var $numeric = FALSE;

  var $group = 0;

  /**
   * Constructor
   */
  function views_term_filter($handler = NULL) {
    if ($handler) {
      $this->set_handler($handler);
    }
  }

  /**
   * Get some information from the handler.
   */
  function set_handler($handler) {
    $this->handler = &$handler;
    $this->query = &$handler->query;

    $this->base_relationship = $handler->relationship;
    $this->base_table = $handler->table;
    $this->field = $handler->field;

    if (!empty($handler->options['group'])) {
      $this->group = $handler->options['group'];
    }

    if (isset($handler->definition['hierarchy table'])) {
      $this->hierarchy_table = $handler->definition['hierarchy table'];
      $this->hierarchy_join = views_get_table_join($this->hierarchy_table, $this->hierarchy_table);
      $this->base_join = views_get_table_join($this->base_table, $this->hierarchy_table);
    }

    if (!empty($handler->definition['numeric'])) {
      $this->placeholder = '%d';
      $this->numeric = TRUE;
    }

    // Discover if we need these:
    if (isset($handler->options['depth'])) {
      $this->depth = $handler->options['depth'];
    }

    if (isset($handler->value) && is_array($handler->value)) {
      $this->value = $handler->value;
    }

    if (isset($handler->operator)) {
      $this->operator = $handler->operator;
    }
  }

  /**
   * This is separate so that it can be easily overridden for really complex
   * relationships. For example, taxonomy's is two layer.
   *
   * This needs a diagram to properly explain.
   */
  function add_join_chain($chain = '', $start_join = NULL, $hierarchy_join = NULL, $base_join = NULL) {
    $query = &$this->query;
    if (!isset($hierarchy_join)) {
      $hierarchy_join = $this->hierarchy_join;
    }

    if (!isset($base_join)) {
      $base_join = $this->base_join;
    }

    $alias = $chain ? '_' . $chain : '';

    // add base table BASE_0
    $this->base_tables[$chain][0] = $query->add_table($this->base_table, $this->base_relationship, $start_join, $this->base_table . $alias . "_0");

    // If we don't have any depth, we're done. That part's easy.
    if (!$this->depth) {
      return;
    }

    $depth = $this->depth;
    if ($depth < 0) {
      // If negative, swap left and right in the hierarchy join so we can
      // search the other way.
      $new_join = drupal_clone($hierarchy_join);
      $temp = $new_join->left_table;
      $new_join->left_table = $new_join->table;
      $new_join->table = $temp;
      $hierarchy_join = $new_join;

      $depth *= -1; // make it positive.
    }

    // @todo add hierarchy BASE_0 --> H_0
    // This adds our first hierarchy table attached to the base table.
    if ($this->hierarchy_table) {
      // First, go with the standard way to link the hierarchy to the base.
      // @todo should this also be passed in? I'm not sure it's ever going to be
      // different.
      $join = views_get_table_join($this->hierarchy_table, $this->base_table);
      $this->hierarchy_tables[$chain][0] = $query->add_relationship($this->hierarchy_table . $alias . "_0", $join, $this->hierarchy_table, $this->base_relationship);

    }

    // It's worth noting that the above is reversed from what you see below, because
    // we start everything with a base table; but the hierarchy wants to start with
    // the hierarchy table and end with the base table. So above, we take our base
    // table and link a hierarchy table onto it. Whereas below, we link our hierarchy
    // table onto the last hierarchy table, and then join a base table off of that.

    // Now, step through our depth and add one of each.
    foreach (range(1, $depth) as $i) {
      if ($this->hierarchy_table) {
        // @todo add H_($i - 1) --> H_$i
        // This relates a new hierarchy table to the last one we added.
        $this->hierarchy_tables[$chain][$i] = $query->add_relationship($this->hierarchy_table . $alias . "_$i", $hierarchy_join, $this->hierarchy_table, $this->hierarchy_tables[$chain][$i - 1]);

        // @todo add H_$i --> BASE_$i
        // This relates a new base table to the hierarchy table we just added.
        $this->base_tables[$chain][$i] = $query->add_relationship($this->base_table . $alias . "_$i", $base_join, $this->base_table, $this->hierarchy_tables[$chain][$i]);
      }
      else {
        // In this case, we just have base tables that link to themselves.
        // @todo fill this in if we ever actually need it.
      }
    }
  }

  function add_filter() {
    switch ($this->operator) {
      case 'or':
        // For or, we add a single join chain.
        $this->add_join_chain(0);
        $placeholders = implode(', ', array_fill(0, count($this->value), $this->placeholder));
        $args = array();
        $clauses = array();
        foreach ($this->base_tables[0] as $table) {
          $args = array_merge($args, $this->value);
          $clauses[] = "$table.$this->field IN ($placeholders)";
        }
        $this->query->add_where($this->group, implode("\n  OR ", $clauses), $args);
        break;
      case 'and':
        // This essentially builds a matrix, and one item in each row
        // row of the matrix must match.
        $clauses = array();
        $args = array();
        foreach ($this->value as $value) {
          // we have to add a join chain for each term we are checking:
          $this->add_join_chain($value);
          $inner = array();
          foreach ($this->base_tables[$value] as $table) {
            $inner[] = "$table.$this->field = $this->placeholder";
            $args[] = $value;
          }
          $clauses[] = '(' . implode("\n  OR ", $inner) . ')';
        }
        $this->query->add_where($this->group, implode("\n  AND ", $clauses), $args);
        break;
      case 'not':
        // This one builds a large matrix and absolutely nothing in the matrix
        // must match.
        $clauses = array();
        $args = array();
        $placeholders = implode(', ', array_fill(0, count($this->terms), $this->placeholder));
        foreach ($this->value as $value) {
          // we have to add a join chain for each value we are checking:
          $this->add_join_chain($value);
          $inner = array();
          foreach ($this->base_tables[$value] as $table) {
            $args = array_merge($args, $this->values);
            $clauses[] = "$table.$this->field NOT IN ($placeholders)";
          }
          $clauses[] = implode("\n  AND ", $inner);
        }
        $this->query->add_where($this->group, implode("\n  AND ", $clauses), $args);
        break;
    }
  }
}

/*
 * Break x,y,z and x+y+z into an array. Numeric only.
 *
 * @param $str
 *   The string to parse.
 * @param $filter
 *   The filter object to use as a base. If not specified one will
 *   be created.
 *
 * @return $filter
 *   The new filter object.
 */
function views_break_phrase($str, $filter = NULL) {
  if (!$filter) {
    $filter = new views_term_filter();
  }
  if (preg_match('/^([0-9]+[+ ])+[0-9]+$/', $str)) {
    // The '+' character in a query string may be parsed as ' '.
    $filter->operator = 'or';
    $filter->terms = preg_split('/[+ ]/', $str);
  }
  else if (preg_match('/^([0-9]+,)*[0-9]+$/', $str)) {
    $filter->operator = 'and';
    $filter->terms = explode(',', $str);
  }

  return $filter;
}

views_include('join.handlers');
views_include('relationship.handlers');
views_include('field.handlers');
views_include('sort.handlers');
views_include('filter.handlers');
views_include('argument.handlers');