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
 * @defgroup views_join_handlers Views' join handlers
 * @{
 * Handlers to tell Views how to join tables together.

 * Here is how you do complex joins:
 *
 * @code
 * class views_join_complex extends views_join {
 *   // PHP 4 doesn't call constructors of the base class automatically from a
 *   // constructor of a derived class. It is your responsibility to propagate
 *   // the call to constructors upstream where appropriate.
 *   function construct($table, $left_table, $left_field, $field, $extra = array(), $type = 'LEFT') {
 *     parent::construct($table, $left_table, $left_field, $field, $extra, $type);
 *   }
 *
 *   function join($table, &$query) {
 *     $output = parent::join($table, $query);
 *   }
 *   $output .= "AND foo.bar = baz.boing";
 *   return $output;
 * }
 * @endcode
 */
/**
 * A function class to represent a join and create the SQL necessary
 * to implement the join.
 *
 * This is the Delegation pattern. If we had PHP5 exclusively, we would
 * declare this an interface.
 *
 * Extensions of this class can be used to create more interesting joins.
 *
 * join definition
 *   - table: table to join (right table)
 *   - field: field to join on (right field)
 *   - left_table: The table we join to
 *   - left_field: The field we join to
 *   - type: either LEFT (default) or INNER
 *   - extra: Either a string that's directly added, or an array of items:
 *   - - table: if not set, current table; if NULL, no table. This field can't
 *       be set in the cached definition because it can't know aliases; this field
 *       can only be used by realtime joins.
 *   - - field: Field or formula
 *   - - operator: defaults to =
 *   - - value: Must be set. If an array, operator will be defaulted to IN.
 *   - - numeric: If true, the value will not be surrounded in quotes.
 *   - extra type: How all the extras will be combined. Either AND or OR. Defaults to AND.
 */
class views_join {
  /**
   * Construct the views_join object.
   */
  function construct($table = NULL, $left_table = NULL, $left_field = NULL, $field = NULL, $extra = array(), $type = 'LEFT') {
    $this->extra_type = 'AND';
    if (!empty($table)) {
      $this->table = $table;
      $this->left_table = $left_table;
      $this->left_field = $left_field;
      $this->field = $field;
      $this->extra = $extra;
      $this->type = strtoupper($type);
    }
    else if (!empty($this->definition)) {
      // if no arguments, construct from definition.
      // These four must exist or it will throw notices.
      $this->table = $this->definition['table'];
      $this->left_table = $this->definition['left_table'];
      $this->left_field = $this->definition['left_field'];
      $this->field = $this->definition['field'];
      if (!empty($this->definition['extra'])) {
        $this->extra = $this->definition['extra'];
      }
      if (!empty($this->definition['extra type'])) {
        $this->extra = strtoupper($this->definition['extra_type']);
      }

      $this->type = !empty($this->definition['type']) ? strtoupper($this->definition['type']) : 'LEFT';
    }
  }

  /**
   * Build the SQL for the join this object represents.
   */
  function join($table, &$query) {
    $left = $query->get_table_info($this->left_table);
    $output = " $this->type JOIN {" . $this->table . "} $table[alias] ON $left[alias].$this->left_field = $table[alias].$this->field";

    // Tack on the extra.
    if (isset($this->extra)) {
      if (is_array($this->extra)) {
        $extras = array();
        foreach ($this->extra as $info) {
          $extra = '';
          // Figure out the table name. Remember, only use aliases provided
          // if at all possible.
          $join_table = '';
          if (!array_key_exists('table', $info)) {
            $join_table = $table['alias'] . '.';
          }
          elseif (isset($info['table'])) {
            $join_table = $info['table'] . '.';
          }

          // And now deal with the value and the operator
          $value = $info['value'];
          if (is_array($value) && !empty($value)) {
            $extra = "$join_table$info[field] IN ('". implode("','", $value) ."')";
          }
          else {
            $operator = "=";
            if (!empty($info['operator'])) {
              $operator = $info['operator'];
            }

            if (empty($info['numeric'])) {
              $extra .= "$join_table$info[field] $operator '$value'";
            }
            else {
              $extra .= "$join_table$info[field] $operator $value";
            }
          }
          $extras[] = $extra;
        }
        if ($extras) {
          $output .= ' AND (' . implode(' ' . $this->extra_type . ' ', $extras) . ')';
        }
      }
      else if ($this->extra && is_string($this->extra)) {
        $output .= " AND ($this->extra)";
      }
    }
    return $output;
  }
}

/**
 * @}
 */

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
 * @defgroup views_relationship_handlers Views' relationship handlers
 * @{
 * Handlers to tell Views how to create alternate relationships.
 */

/**
 * Simple relationship handler that allows a new version of the primary table
 * to be linked in.
 */
class views_handler_relationship extends views_handler {
  /**
   * Get this field's label.
   */
  function label() {
    if (!isset($this->options['label'])) {
      return $this->ui_name();
    }
    return $this->options['label'];
  }

  /**
   * Provide a default label
   */
  function options(&$options) {
    parent::options($options);
    $options['label'] = !empty($this->definition['label']) ? $this->definition['label'] : $this->definition['field'];
  }

  /**
   * Default options form that provides the label widget that all fields
   * should have.
   */
  function options_form(&$form, &$form_state) {
    $form['label'] = array(
      '#type' => 'textfield',
      '#title' => t('Label'),
      '#default_value' => isset($this->options['label']) ? $this->options['label'] : '',
      '#description' => t('The label for this relationship that will be displayed only administratively.'),
    );
  }

  /**
   * Called to implement a relationship in a query.
   */
  function query() {
    // Figure out what base table this relationship brings to the party.
    $table_data = views_fetch_data($this->definition['base']);
    $base_field = $table_data['table']['base']['field'];

    $def = $this->definition;
    $def['table'] = $this->definition['base'];
    $def['field'] = $base_field;
    $def['left_table'] = $this->table;
    $def['left_field'] = $this->real_field;
    if (!empty($def['join_handler']) && class_exists($def['join_handler'])) {
      $join = new $def['join_handler'];
    }
    else {
      $join = new views_join();
    }

    $join->definition = $def;
    $join->construct();

    $this->ensure_my_table();

    // use a short alias for this:
    $alias = $this->options['id'];

    $this->alias = $this->query->add_relationship($alias, $join, $this->definition['base'], $this->relationship);
  }
}

/**
 * @}
 */

/**
 * @defgroup views_field_handlers Views' field handlers
 * @{
 * Handlers to tell Views how to build and display fields.
 *
 */

/**
 * Base field handler that has no options and renders an unformatted field.
 */
class views_handler_field extends views_handler {
  var $field_alias = 'unknown';

  /**
   * Construct a new field handler.
   */
  function construct() {
    $this->additional_fields = array();
    if (!empty($this->definition['additional fields'])) {
      $this->additional_fields = $this->definition['additional fields'];
    }
  }

  /**
   * Called to add the field to a query.
   */
  function query() {
    $this->ensure_my_table();
    // Add the field.
    $this->field_alias = $this->query->add_field($this->table_alias, $this->real_field);

    $this->add_additional_fields();
  }

  /**
   * Add 'additional' fields to the query.
   *
   * @param $fields
   * An array of fields. The key is an identifier used to later find the
   * field alias used. The value is either a string in which case it's
   * assumed to be a field on this handler's table; or it's an array in the
   * form of
   * @code array('table' => $tablename, 'field' => $fieldname) @endcode
   */
  function add_additional_fields($fields = NULL) {
    if (!isset($fields)) {
      // notice check
      if (empty($this->additional_fields)) {
        return;
      }
      $fields = $this->additional_fields;
    }
    if (!empty($fields) && is_array($fields)) {
      foreach ($fields as $identifier => $info) {
        if (is_array($info)) {
          if (isset($info['table'])) {
            $table_alias = $this->query->ensure_table($info['table'], $this->relationship);
          }
          else {
            $table_alias = $this->table_alias;
          }
          $this->aliases[$identifier] = $this->query->add_field($table_alias, $info['field']);
        }
        else {
          $this->aliases[$info] = $this->query->add_field($this->table_alias, $info);
        }
      }
    }
  }

  /**
   * Called to determine what to tell the clicksorter.
   */
  function click_sort($order) {
    $this->query->add_orderby($this->table, $this->field, $order, $this->field_alias);
  }

  /**
   * Determine if this field is click sortable.
   */
  function click_sortable() {
    return !empty($this->definition['click sortable']);
  }

  /**
   * Get this field's label.
   */
  function label() {
    if (!isset($this->options['label'])) {
      return '';
    }
    return $this->options['label'];
  }

  /**
   * Provide a default label
   */
  function options(&$options) {
    parent::options($options);
    $options['label'] = $this->ui_name();
  }

  /**
   * Default options form that provides the label widget that all fields
   * should have.
   */
  function options_form(&$form, &$form_state) {
    $form['label'] = array(
      '#type' => 'textfield',
      '#title' => t('Label'),
      '#default_value' => isset($this->options['label']) ? $this->options['label'] : '',
      '#description' => t('The label for this field that will be displayed to end users if the style requires it.'),
    );
  }

  /**
   * Provide extra data to the administration form
   */
  function admin_summary() {
    return $this->label();
  }

  /**
   * Run before any fields are rendered.
   *
   * This gives the handlers some time to set up before any handler has
   * been rendered.
   *
   * @param $values
   *   An array of all objects returned from the query.
   */
  function pre_render($values) { }

  /**
   * Render the field.
   *
   * @param $values
   *   The values retrieved from the database.
   */
  function render($values) {
    $value = $values->{$this->field_alias};
    return check_plain($value);
  }
}

/**
 * A handler to provide proper displays for dates.
 */
class views_handler_field_date extends views_handler_field {
  /**
   * Fill in default options.
   */
  function options(&$options) {
    parent::options($options);
    $options['date_format'] = 'small';
    $options['custom_date_format'] = '';
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $time = time();

    $form['date_format'] = array(
      '#type' => 'select',
      '#title' => t('Date format'),
      '#options' => array(
        'small' => format_date($time, 'small'),
        'medium' => format_date($time, 'medium'),
        'large' => format_date($time, 'large'),
        'custom' => t('Custom'),
        'time ago' => t('Time ago'),
      ),
      '#default_value' => isset($this->options['date_format']) ? $this->options['date_format'] : 'small',
    );
    $form['custom_date_format'] = array(
      '#type' => 'textfield',
      '#title' => t('Custom date format'),
      '#description' => t('If "Custom", see <a href="http://us.php.net/manual/en/function.date.php">the PHP docs</a> for date formats. If "Time ago" this is the the number of different units to display, which defaults to two.'),
      '#default_value' => isset($this->options['custom_date_format']) ? $this->options['custom_date_format'] : '',
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-date-format' => array('custom', 'time ago')),
    );
  }

  function render($values) {
    $value = $values->{$this->field_alias};
    $format = $this->options['date_format'];
    if ($format == 'custom') {
      $custom_format = $this->custom_date_format;
    }

    switch ($format) {
      case 'time ago':
        return $value ? t('%time ago', array('%time' => format_interval(time() - $value, is_numeric($custom_format) ? $custom_format : 2))) : theme('views_nodate');
      case 'custom':
        return $value ? format_date($value, $format, $custom_format) : theme('views_nodate');
      default:
        return $value ? format_date($value, $format) : theme('views_nodate');
    }
  }
}

/**
 * A handler to provide proper displays for dates.
 *
 * Allows for display of true/false, yes/no, on/off.
 */
class views_handler_field_boolean extends views_handler_field {
  function options(&$options) {
    parent::options($options);
    $options['type'] = 'yes-no';
    $options['not'] = FALSE;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['type'] = array(
      '#type' => 'select',
      '#title' => t('Output format'),
      '#options' => array(
        'yes-no' => t('Yes/No'),
        'true-false' => t('True/False'),
        'on-off' => t('On/Off'),
      ),
      '#default_value' => $this->options['type'],
    );
    $form['not'] = array(
      '#type' => 'checkbox',
      '#title' => t('Reverse'),
      '#description' => t('If checked, true will be displayed as false.'),
      '#default_value' => $this->options['not'],
    );
  }

  function render($values) {
    $value = $values->{$this->field_alias};
    if (!empty($this->options['not'])) {
      $value = !$value;
    }

    switch ($this->options['type']) {
      case 'yes-no':
      default:
        return $value ? t('Yes') : t('No');
      case 'true-false':
        return $value ? t('True') : t('False');
      case 'on-off':
        return $value ? t('On') : t('Off');
    }
  }
}

/**
 * A handler to run a field through check_markup, using a companion
 * format field.
 */
class views_handler_field_markup extends views_handler_field {
  /**
   * Constructor; calls to base object constructor.
   */
  function construct() {
    $this->format = $this->definition['format'];

    $this->additional_fields = array();
    if (!is_numeric($this->format)) {
      $this->additional_fields['format'] = $this->format;
    }
  }

  function render($values) {
    $value = $values->{$this->field_alias};
    $format = is_numeric($this->format) ? $this->format : $values->{$this->aliases['format']};
    return check_markup($value, $format, FALSE);
  }
}

/**
 * A handler to run a field through simple XSS filtering
 */
class views_handler_field_xss extends views_handler_field {
  function render($values) {
    $value = $values->{$this->field_alias};
    return filter_xss($value);
  }
}

/**
 * Field handler to provide simple renderer that turns a URL into a clickable link.
 */
class views_handler_field_url extends views_handler_field {
  /**
   * Provide link to the page being visited.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['display_as_link'] = array(
      '#title' => t('Display as link'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['display_as_link']),
    );
  }

  function render($values) {
    $value = $values->{$this->field_alias};
    if (!empty($this->options['display_as_link'])) {
      return l(check_plain($value), $value, array('html' => TRUE));
    }
    else {
      return $value;
    }
  }
}

/**
 * @}
 */

/**
 * @defgroup views_sort_handlers Views' sort handlers
 * @{
 * Handlers to tell Views how to sort queries
 */
/**
 * Base sort handler that has no options and performs a simple sort
 */
class views_handler_sort extends views_handler {
  /**
   * Called to add the sort to a query.
   */
  function query() {
    $this->ensure_my_table();
    // Add the field.
    $this->query->add_orderby($this->table_alias, $this->real_field, $this->options['order']);
  }

  /**
   * Set defaults for new handler
   */
  function options(&$options) {
    parent::options($options);
    $options['order'] = 'ASC';
  }

  /**
   * Display whether or not the sort order is ascending or descending
   */
  function admin_summary() {
    switch ($this->options['order']) {
      case 'ASC':
      case 'asc':
      default:
        $type = t('asc');
        break;
      case 'DESC';
      case 'desc';
        $type = t('desc');
        break;
    }
    return '<span class="views-ascending"><span>' . $type . '</span></span>';
  }

  /**
   * Basic options for all sort criteria
   */
  function options_form(&$form, &$form_state) {
    $form['order'] = array(
      '#type' => 'radios',
      '#title' => t('Sort order'),
      '#options' => array('ASC' => t('Ascending'), 'DESC' => t('Descending')),
      '#default_value' => $this->options['order'],
    );
  }
}

/**
 * Base sort handler that has no options and performs a simple sort
 */
class views_handler_sort_formula extends views_handler_sort {
  /**
   * Constructor to take the formula this sorts on.
   */
  function construct() {
    $this->formula = $this->definition['formula'];
    if (is_array($this->formula) && !isset($this->formula['default'])) {
      $this->error = t('views_handler_sort_formula missing default: @formula', array('@formula' => var_export($this->formula, TRUE)));
    }
    parent::construct();
  }
  /**
   * Called to add the sort to a query.
   */
  function query() {
    if (is_array($this->formula)) {
      global $db_type;
      if (isset($this->formula[$db_type])) {
        $formula = $this->formula[$db_type];
      }
      else {
        $formula = $this->formula['default'];
      }
    }
    else {
      $formula = $this->formula;
    }
    $this->ensure_my_table();
    // Add the field.
    $this->query->add_orderby(NULL, $formula, $this->options['order'], $this->table_alias . '_' . $this->field);
  }
}

class views_handler_sort_date extends views_handler_sort {
  function options(&$options) {
    parent::options($options);
    $options['granularity'] = 'second';
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['granularity'] = array(
      '#type' => 'radios',
      '#title' => t('Granularity'),
      '#options' => array(
        'second' => t('Second'),
        'minute' => t('Minute'),
        'hour'   => t('Hour'),
        'day'    => t('Day'),
        'month'  => t('Month'),
        'year'   => t('Year'),
      ),
      '#description' => t('The granularity is the smallest unit to use when determining whether two dates are the same; for example, if the granularity is "Year" then all dates in 1999, regardless of when they fall in 1999, will be considered the same date.'),
      '#default_value' => $this->options['granularity'],
    );
  }

  /**
   * Called to add the sort to a query.
   */
  function query() {
    $this->ensure_my_table();
    switch($this->options['granularity']) {
      case 'second':
      default:
        $this->query->add_orderby($this->table_alias, "$this->table_alias.$this->real_field", $this->options['order']);
        return;
      case 'minute':
        $formula = views_date_sql_format('YmdHi', "$this->table_alias.$this->real_field");
        break;
      case 'hour':
        $formula = views_date_sql_format('YmdH', "$this->table_alias.$this->real_field");
        break;
      case 'day':
        $formula = views_date_sql_format('Ymd', "$this->table_alias.$this->real_field");
        break;
      case 'month':
        $formula = views_date_sql_format('Ym', "$this->table_alias.$this->real_field");
        break;
      case 'year':
        $formula = views_date_sql_format('Y', "$this->table_alias.$this->real_field");
        break;
    }

    // Add the field.
    $this->query->add_orderby(NULL, $formula, $this->options['order'], $this->table_alias . '_' . $this->field . '_' . $this->options['granularity']);
  }
}


/**
 * @}
 */

/**
 * @defgroup views_filter_handlers Views' filter handlers
 * @{
 * Handlers to tell Views how to filter queries.
 */

/**
 * Base class for filters.
 */
class views_handler_filter extends views_handler {
  /**
   * Provide some extra help to get the operator/value easier to use.
   *
   * This likely has to be overridden by filters which are more complex
   * than simple operator/value.
   */
  function init(&$view, $options) {
    parent::init($view, $options);

    $this->operator = $options['operator'];
    $this->value = $options['value'];
  }

  /**
   * Provide a simple default initializer -- should be overridden.
   */
  function options(&$options) {
    parent::options($options);
    $options['operator'] = '=';
    $options['value'] = '';
    $options['group'] = 0;
    $options['exposed'] = FALSE;
    $options['expose'] = array(
      'operator' => FALSE,
      'label' => '',
    );
  }

  /**
   * Display the filter on the administrative summary
   */
  function admin_summary() {
    return check_plain($this->operator) . ' ' . check_plain($this->value);
  }

  /**
   * Provide the basic form which calls through to subforms.
   * If overridden, it is best to call through to the parent,
   * or to at least make sure all of the functions in this form
   * are called.
   */
  function options_form(&$form, &$form_state) {
    $this->show_expose_button($form, $form_state);
    $this->show_operator_form($form, $form_state);
    $this->show_value_form($form, $form_state);
    $this->show_expose_form($form, $form_state);
  }

  /**
   * Simple validate handler
   */
  function options_validate(&$form, &$form_state) {
    $this->operator_validate($form, $form_state);
    $this->value_validate($form, $form_state);
    if (!empty($this->options['exposed'])) {
      $this->expose_validate($form, $form_state);
    }

  }

  /**
   * Simple submit handler
   */
  function options_submit(&$form, &$form_state) {
    $this->operator_submit($form, $form_state);
    $this->value_submit($form, $form_state);
    if (!empty($this->options['exposed'])) {
      $this->expose_submit($form, $form_state);
    }
  }

  /**
   * Shortcut to display the operator form.
   */
  function show_operator_form(&$form, &$form_state) {
    $this->operator_form($form, $form_state);
    $form['operator']['#prefix'] = '<div class="views-left-30">';
    $form['operator']['#suffix'] = '</div>';
  }

  /**
   * Provide a form for setting the operator.
   *
   * This may be overridden by child classes, and it must
   * define $form['operator'];
   */
  function operator_form(&$form, &$form_state) {
    $options = $this->operator_options();
    $form['operator'] = array(
      '#type' => count($options) < 10 ? 'radios' : 'select',
      '#title' => t('Operator'),
      '#default_value' => $this->operator,
      '#options' => $options,
    );
  }

  /**
   * Provide a list of options for the default operator form.
   * Should be overridden by classes that don't override operator_form
   */
  function operator_options() { return array(); }

  /**
   * Validate the operator form.
   */
  function operator_validate($form, &$form_state) { }

  /**
   * Perform any necessary changes to the form values prior to storage.
   * There is no need for this function to actually store the data.
   */
  function operator_submit($form, &$form_state) { }

  /**
   * Shortcut to display the value form.
   */
  function show_value_form(&$form, &$form_state) {
    $this->value_form($form, $form_state);
    $form['value']['#prefix'] = '<div class="views-left-60">';
    $form['value']['#suffix'] = '</div>';
  }

  /**
   * Provide a form for setting options.
   *
   * This should be overridden by all child classes and it must
   * define $form['value']
   */
  function value_form(&$form, &$form_state) { $form['value'] = array(); }

  /**
   * Validate the options form.
   */
  function value_validate($form, &$form_state) { }

  /**
   * Perform any necessary changes to the form values prior to storage.
   * There is no need for this function to actually store the data.
   */
  function value_submit($form, &$form_state) { }

  /**
   * Shortcut to display the expose/hide button.
   */
  function show_expose_button(&$form, &$form_state) {
    $form['expose_button'] = array(
      '#prefix' => '<div class="views-expose clear-block">',
      '#suffix' => '</div>',
    );
    if (empty($this->options['exposed'])) {
      $form['expose_button']['button'] = array(
        '#type' => 'submit',
        '#value' => t('Expose'),
        '#submit' => array('views_ui_config_item_form_expose'),
      );
      $form['expose_button']['markup'] = array(
        '#prefix' => '<div class="description">',
        '#value' => t('This item is currently not exposed. If you <strong>expose</strong> it, users will be able to change the filter as they view it.'),
        '#suffix' => '</div>',
      );
    }
    else {
      $form['expose_button']['button'] = array(
        '#type' => 'submit',
        '#value' => t('Hide'),
        '#submit' => array('views_ui_config_item_form_expose'),
      );
      $form['expose_button']['markup'] = array(
        '#prefix' => '<div class="description">',
        '#value' => t('This item is currently exposed. If you <strong>hide</strong> it, users will not able to change the filter as they view it.'),
        '#suffix' => '</div>',
      );
    }
  }

  /**
   * Shortcut to display the exposed options form.
   */
  function show_expose_form(&$form, &$form_state) {
    if (empty($this->options['exposed'])) {
      return;
    }

    $form['expose'] = array(
      '#prefix' => '<div class="views-expose-options clear">',
      '#suffix' => '</div>',
    );
    $this->expose_form($form, $form_state);
  }

  /**
   * Overridable form for exposed filter options.
   *
   * If overridden, it is best to call the parent or re-implement
   * the stuff here.
   *
   * Many filters will need to override this in order to provide options
   * that are nicely tailored to the given filter.
   */
  function expose_form(&$form, &$form_state) {
    // @todo we should break this up into two functions to make it easier
    // for child objects to put options in the left or right side without
    // having to override this whole thing.
    $form['expose']['start_left'] = array(
      '#value' => '<div class="views-left-50">',
    );

    if (!empty($form['operator']['#type'])) {
      $form['expose']['operator'] = array(
        '#type' => 'textfield',
        '#default_value' => $this->options['expose']['operator'],
        '#title' => t('Operator identifier'),
        '#size' => 40,
        '#description' => t('This will appear in the URL after the ? to identify this operator. Leave blank to not expose the operator.'),
      );
    }
    else {
      $form['expose']['operator'] = array(
        '#type' => 'value',
        '#value' => '',
      );
    }
    $form['expose']['identifier'] = array(
      '#type' => 'textfield',
      '#default_value' => $this->options['expose']['identifier'],
      '#title' => t('Filter identifier'),
      '#size' => 40,
      '#description' => t('This will appear in the URL after the ? to identify this filter. Cannot be blank.'),
    );
    $form['expose']['label'] = array(
      '#type' => 'textfield',
      '#default_value' => $this->options['expose']['label'],
      '#title' => t('Label'),
      '#size' => 40,
    );

    $form['expose']['end_left'] = array(
      '#value' => '</div>',
    );

    $form['expose']['start_checkboxes'] = array(
      '#value' => '<div class="form-checkboxes views-left-40 clear-block">',
    );
    $form['expose']['optional'] = array(
      '#type' => 'checkbox',
      '#title' => t('Optional'),
      '#description' => t('This exposed filter is optional and will have added options to allow it not to be set.'),
      '#default_value' => $this->options['expose']['optional'],
    );
    if (empty($this->no_single)) {
      $form['expose']['single'] = array(
        '#type' => 'checkbox',
        '#title' => t('Force single'),
        '#description' => t('Force this exposed filter to accept only one option.'),
        '#default_value' => $this->options['expose']['single'],
      );
    }
    $form['expose']['remember'] = array(
      '#type' => 'checkbox',
      '#title' => t('Remember'),
      '#description' => t('Remember the last setting the user gave this filter.'),
      '#default_value' => $this->options['expose']['remember'],
    );
    $form['expose']['end_checkboxes'] = array(
      '#value' => '</div>',
    );
  }

  /**
   * Validate the options form.
   */
  function expose_validate($form, &$form_state) {
    if (empty($this->options['expose']['identifier'])) {
      if (empty($form_state['values']['options']['expose']['identifier'])) {
        form_error($form['expose']['identifier'], t('The identifier is required if the filter is
        exposed.'));
      }
    }
  }

  /**
   * Perform any necessary changes to the form exposes prior to storage.
   * There is no need for this function to actually store the data.
   */
  function expose_submit($form, &$form_state) { }

  function expose_options() {
    $this->options['expose'] = array(
      'operator' => $this->options['id'] . '_oper',
      'identifier' => $this->options['id'],
      'label' => $this->ui_name(),
      'remember' => FALSE,
      'single' => TRUE,
      'optional' => TRUE,
    );
  }
  /**
   * Render our chunk of the exposed filter form when selecting
   *
   * You can override this if it doesn't do what you expect.
   */
  function exposed_form(&$form, &$form_state) {
    if (empty($this->options['exposed'])) {
      return;
    }

    if (!empty($this->options['expose']['operator'])) {
      $operator = $this->options['expose']['operator'];
      $this->operator_form($form, $form_state);
      $form[$operator] = $form['operator'];

      if (isset($form[$operator]['#title'])) {
        unset($form[$operator]['#title']);
      }

      $this->exposed_translate($form[$operator], 'operator');

      unset($form['operator']);
    }

    if (!empty($this->options['expose']['identifier'])) {
      $value = $this->options['expose']['identifier'];
      $this->value_form($form, $form_state);
      $form[$value] = $form['value'];

      if (isset($form[$value]['#title']) && !empty($form[$value]['#type']) && $form[$value]['#type'] != 'checkbox') {
        unset($form[$value]['#title']);
      }

      $this->exposed_translate($form[$value], 'value');

      if (!empty($form['#type']) && ($form['#type'] == 'checkboxes' || ($form['#type'] == 'select' && !empty($form['#multiple'])))) {
        unset($form[$value]['#default_value']);
      }

      if (!empty($form['#type']) && $form['#type'] == 'select' && empty($form['#multiple'])) {
        $form[$value]['#default_value'] = 'All';
      }

      unset($form['value']);
    }
  }

  /**
   * Make some translations to a form item to make it more suitable to
   * exposing.
   */
  function exposed_translate(&$form, $type) {
    if (!isset($form['#type'])) {
      return;
    }

    if ($form['#type'] == 'radios') {
      $form['#type'] = 'select';
    }
    if ($form['#type'] == 'checkboxes' && !empty($this->options['expose']['single'])) {
      $form['#type'] = 'select';
    }
    if (!empty($this->options['expose']['single']) && isset($form['#multiple'])) {
      unset($form['#multiple']);
    }

    if ($type == 'value' && !empty($this->options['expose']['optional']) && $form['#type'] == 'select' && empty($form['#multiple'])) {
      $form['#options'] = array('All' => t('<Any>')) + $form['#options'];
      $form['#default_value'] = 'All';
    }
  }

  /**
   * Tell the renderer about our exposed form. This only needs to be
   * overridden for particularly complex forms. And maybe not even then.
   */
  function exposed_info() {
    if (empty($this->options['exposed'])) {
      return;
    }

    return array(
      'operator' => $this->options['expose']['operator'],
      'value' => $this->options['expose']['identifier'],
      'label' => $this->options['expose']['label'],
    );
  }

  /**
   * Check to see if input from the exposed filters should change
   * the behavior if this filter.
   */
  function accept_exposed_input($input) {
    if (empty($this->options['exposed'])) {
      return TRUE;
    }

    if (!empty($this->options['expose']['operator']) && isset($input[$this->options['expose']['operator']])) {
      $this->operator = $input[$this->options['expose']['operator']];
      if ($this->options['expose']['remember']) {
        $_SESSION['views'][$this->view->name][$this->view->current_display][$this->options['expose']['operator']] = $input[$this->options['expose']['operator']];
      }
    }

    if (!empty($this->options['expose']['identifier'])) {
      $value = $input[$this->options['expose']['identifier']];
      if ($this->options['expose']['remember']) {
        $_SESSION['views'][$this->view->name][$this->view->current_display][$this->options['expose']['identifier']] = $value;
      }

      // Various ways to check for the absence of optional input.
      if (!empty($this->options['expose']['optional'])) {
        if ($value == 'All' || $value === array()) {
          return FALSE;
        }

        if (!empty($this->no_single) && $value === '') {
          return FALSE;
        }
      }


      if (isset($value)) {
        $this->value = $value;
      }
      else {
        return FALSE;
      }
    }

    return TRUE;
  }

  /**
   * Add this filter to the query.
   *
   * Due to the nature of fapi, the value and the operator have an unintended
   * level of indirection. You will find them in $this->operator
   * and $this->value respectively.
   */
  function query() {
    $this->ensure_my_table();
    $this->query->add_where($this->options['group'], "$this->table_alias.$this->real_field " . $this->operator . " '%s'", $this->value);
  }
}

/**
 * Simple filter to handle equal to / not equal to filters
 */
class views_handler_filter_equality extends views_handler_filter {
  // exposed filter options
  var $no_single = TRUE;

  /**
   * Provide basic defaults for the equality operator
   */
  function options(&$options) {
    parent::options($options);
    $options['operator'] = '=';
    $options['value'] = '';
  }

  /**
   * Provide simple equality operator
   */
  function operator_options() {
    return array(
      '=' => t('Is equal to'),
      '!=' => t('Is not equal to'),
    );
  }

  /**
   * Provide a simple textfield for equality
   */
  function value_form(&$form, &$form_state) {
    $form['value'] = array(
      '#type' => 'textfield',
      '#title' => t('Value'),
      '#size' => 30,
      '#default_value' => $this->value,
    );
  }
}

/**
 * Basic textfield filter to handle string filtering commands
 * including equality, like, not like, etc.
 */
class views_handler_filter_string extends views_handler_filter {
  // exposed filter options
  var $no_single = TRUE;

  /**
   * Provide basic defaults for the equality operator
   */
  function options(&$options) {
    parent::options($options);
    $options['operator'] = '=';
    $options['value'] = '';
    $options['case'] = TRUE;
  }

  /**
   * This kind of construct makes it relatively easy for a child class
   * to add or remove functionality by overriding this function and
   * adding/removing items from this array.
   */
  function operators() {
    $operators = array(
      '=' => array(
        'title' => t('Is equal to'),
        'short' => t('='),
        'method' => 'op_equal',
        'values' => 1,
      ),
      '!=' => array(
        'title' => t('Is not equal to'),
        'short' => t('='),
        'method' => 'op_equal',
        'values' => 1,
      ),
      'contains' => array(
        'title' => t('Contains'),
        'short' => t('contains'),
        'method' => 'op_contains',
        'values' => 1,
      ),
      'word' => array(
        'title' => t('Contains any word'),
        'short' => t('has word'),
        'method' => 'op_word',
        'values' => 1,
      ),
      'allwords' => array(
        'title' => t('Contains all words'),
        'short' => t('has all'),
        'method' => 'op_word',
        'values' => 1,
      ),
      'starts' => array(
        'title' => t('Starts with'),
        'short' => t('begins'),
        'method' => 'op_starts',
        'values' => 1,
      ),
      'ends' => array(
        'title' => t('Ends with'),
        'short' => t('ends'),
        'method' => 'op_ends',
        'values' => 1,
      ),
      'not' => array(
        'title' => t('Does not contain'),
        'short' => t('!has'),
        'method' => 'op_not',
        'values' => 1,
      ),
    );
    // if the definition allows for the empty operator, add it.
    if (!empty($this->definition['allow empty'])) {
      $operators += array(
        'empty' => array(
          'title' => t('Is empty (NULL)'),
          'method' => 'op_empty',
          'short' => t('empty'),
          'values' => 0,
        ),
        'not empty' => array(
          'title' => t('Is not empty (NULL)'),
          'method' => 'op_empty',
          'short' => t('not empty'),
          'values' => 0,
        ),
      );
    }

    return $operators;
  }

  /**
   * Build strings from the operators() for 'select' options
   */
  function operator_options($which = 'title') {
    $options = array();
    foreach ($this->operators() as $id => $info) {
      $options[$id] = $info[$which];
    }

    return $options;
  }

  function admin_summary() {
    $options = $this->operator_options('short');
    return (!empty($this->options['exposed']) ? t('exposed<br />') : '') . $options[$this->operator];
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['case'] = array(
      '#type' => 'checkbox',
      '#title' => t('Case sensitive'),
      '#default_value' => $this->options['case'],
      '#description' => t('Case sensitive filters may be faster; MySQL might ignore case sensitivity.'),
    );
  }

  function operator_values($values = 1) {
    $options = array();
    foreach ($this->operators() as $id => $info) {
      if (isset($info['values']) && $info['values'] == $values) {
        $options[] = $id;
      }
    }

    return $options;
  }

  /**
   * Provide a simple textfield for equality
   */
  function value_form(&$form, &$form_state) {
    // We have to make some choices when creating this as an exposed
    // filter form. For example, if the operator is locked and thus
    // not rendered, we can't render dependencies; instead we only
    // render the form items we need.
    $which = 'all';
    $source = ($form['operator']['#type'] == 'radios') ? 'radio:options[operator]' : 'edit-options-operator';
    if (!empty($form_state['exposed'])) {
      if (empty($this->options['expose']['operator'])) {
        // exposed and locked.
        $which = in_array($this->operator, $this->operator_values(1)) ? 'value' : 'none';
      }
      else {
        $source = 'edit-' . form_clean_id($this->options['expose']['operator']);
      }
    }

    if ($which == 'all' || $which == 'value') {
      $form['value'] = array(
        '#type' => 'textfield',
        '#title' => t('Value'),
        '#size' => 30,
        '#default_value' => $this->value,
      );
      if ($which == 'all') {
        $form['value'] += array(
          '#process' => array('views_process_dependency'),
          '#dependency' => array($source => $this->operator_values(1)),
        );
      }
    }
  }

  function case_transform() {
    return empty($this->options['case']) ? '' : 'UPPER';
  }

  /**
   * Add this filter to the query.
   *
   * Due to the nature of fapi, the value and the operator have an unintended
   * level of indirection. You will find them in $this->operator
   * and $this->value respectively.
   */
  function query() {
    $this->ensure_my_table();
    $field = "$this->table_alias.$this->real_field";
    $upper = $this->case_transform();

    $info = $this->operators();
    if (!empty($info[$this->operator]['method'])) {
      $this->{$info[$this->operator]['method']}($field, $upper);
    }
  }

  function op_equal($field, $upper) {
    // operator is either = or !=
    $this->query->add_where($this->options['group'], "$upper(%s) $this->operator $upper('%s')", $field, $this->value);
  }

  function op_contains($field, $upper) {
    $this->query->add_where($this->options['group'], "$upper(%s) LIKE $upper('%%%s%%')", $field, $this->value);
  }

  function op_word($field, $upper) {
    preg_match_all('/ (-?)("[^"]+"|[^" ]+)/i', ' '. $this->value, $matches, PREG_SET_ORDER);
    foreach ($matches as $match) {
      $phrase = false;
      // Strip off phrase quotes
      if ($match[2]{0} == '"') {
        $match[2] = substr($match[2], 1, -1);
        $phrase = true;
      }
      $words = trim($match[2], ',?!();:-');
      $words = $phrase ? array($words) : preg_split('/ /', $words, -1, PREG_SPLIT_NO_EMPTY);
      foreach ($words as $word) {
        $where[] = "$upper(%s) LIKE $upper('%%%s%%')";
        $values[] = $field;
        $values[] = trim($word, " ,!?");
      }
    }
    if ($this->operator == 'word') {
      $where = '('. implode(' OR ', $where) .')';
    }
    else {
      $where = implode(' AND ', $where);
    }
    // previously this was a call_user_func_array but that's unnecessary
    // as views will unpack an array that is a single arg.
    $this->query->add_where($this->options['group'], $where, $values);
  }

  function op_starts($field, $upper) {
    $this->query->add_where($this->options['group'], "$upper(%s) LIKE $upper('%s%%')", $field, $this->value);
  }

  function op_ends($field, $upper) {
    $this->query->add_where($this->options['group'], "$upper(%s) LIKE $upper('%%%s')", $field, $this->value);
  }

  function op_not($field, $upper) {
    $this->query->add_where($this->options['group'], "$upper(%s) NOT LIKE $upper('%%%s%%')", $field, $this->value);
  }

  function op_empty($field) {
    if ($this->operator == 'empty') {
      $operator = "IS NULL";
    }
    else {
      $operator = "IS NOT NULL";
    }

    $this->query->add_where($this->options['group'], "$field $operator");
  }

}


/**
 * Simple filter to handle matching of boolean values
 */
class views_handler_filter_boolean_operator extends views_handler_filter {
  // exposed filter options
  var $no_single = TRUE;

  function construct() {
    $this->value_value = t('True');
    if (isset($this->definition['label'])) {
      $this->value_value = $this->definition['label'];
    }
    parent::construct();
  }

  function options(&$options) {
    parent::options($options);
    $options['value'] = FALSE;
  }

  function operator_form(&$form, &$form_state) {
    $form['operator'] = array();
  }

  function value_form(&$form, &$form_state) {
    if (empty($this->options['exposed'])) {
      $form['value'] = array(
        '#type' => 'checkbox',
        '#title' => $this->value_value,
        '#default_value' => $this->value,
      );
    }
    else {
      $form['value'] = array(
        '#type' => 'select',
        '#title' => $this->value_value,
        '#options' => array(1 => t('Yes'), 0 => t('No')),
        '#default_value' => $this->value,
      );
    }
  }
  function admin_summary() {
    if (!empty($this->options['exposed'])) {
      return t('exposed');
    }

    return (empty($this->value) ? t("False") : t('True'));
  }

  function expose_options() {
    $this->options['expose'] = array(
      'operator' => '',
      'identifier' => $this->options['id'],
      'label' => $this->value_value,
      'remember' => FALSE,
      'single' => TRUE,
      'optional' => FALSE,
    );
  }

  function query() {
    $this->ensure_my_table();
    $this->query->add_where($this->options['group'], "$this->table_alias.$this->real_field " . (empty($this->value) ? '=' : '<>') . " 0");
  }

}

/**
 * Simple filter to handle matching of multiple options selectable via checkboxes
 */
class views_handler_filter_in_operator extends views_handler_filter {
  function construct() {
    parent::construct();
    $this->value_title = t('Options');
    $this->value_options = array(t('Yes'), t('No'));
  }

  function options(&$options) {
    parent::options($options);
    $options['operator'] = 'in';
    $options['value'] = array();
  }

  /**
   * Provide inclusive/exclusive matching
   */
  function operator_options() {
    return array(
      'in' => t('Is one of'),
      'not in' => t('Is not one of'),
    );
  }

  function value_form(&$form, &$form_state) {
    $form['value'] = array(
      '#type' => 'checkboxes',
      '#title' => $this->value_title,
      '#options' => $this->value_options,
      '#default_value' => (array) $this->value,
    );
  }

  function value_submit($form, &$form_state) {
    $form_state['values']['options']['value'] = array_filter($form_state['values']['options']['value']);
  }

  function admin_summary() {
    if (!empty($this->options['exposed'])) {
      return t('exposed');
    }

    if (count($this->value) == 1) {
      // If there is only one, show it as an =.
      $keys = array_keys($this->value);
      $key = array_shift($keys);
      if (!empty($this->value_options[$key])) {
        $value = check_plain($this->value_options[$key]);
      }
      else {
        $value = t('Unknown');
      }

      return ($this->operator == 'in' ? '=' : '<>') . ' ' . $value;
    }
    $output = '';
    foreach ($this->value as $value) {
      if ($output) {
        $output .= ', ';
      }
      if (strlen($output) > 8) {
        $output .= '...';
        break;
      }
      $output .= check_plain($this->value_options[$value]);
    }

    return check_plain($this->operator) . ' ' . $output;
  }

  function query() {
    if (empty($this->value)) {
      return;
    }
    $this->ensure_my_table();
    $replace = array_fill(0, sizeof($this->value), "'%s'");
    $in = ' (' . implode(", ", $replace) . ')';
    $this->query->add_where($this->options['group'], "$this->table_alias.$this->real_field " . $this->operator . $in, $this->value);
  }
}

/**
 * Simple filter to handle greater than/less than filters
 */
class views_handler_filter_numeric extends views_handler_filter {
  var $no_single = TRUE;
  /**
   * Provide basic defaults for the filter
   */
  function options(&$options) {
    parent::options($options);
    $options['operator'] = '=';
    $options['value']['min'] = '';
    $options['value']['max'] = '';
    $options['value']['value'] = '';
  }

  function operators() {
    $operators = array(
      '<' => array(
        'title' => t('Is less than'),
        'method' => 'op_simple',
        'short' => t('<'),
        'values' => 1,
      ),
      '<=' => array(
        'title' => t('Is less than or equal to'),
        'method' => 'op_simple',
        'short' => t('<='),
        'values' => 1,
      ),
      '=' => array(
        'title' => t('Is equal to'),
        'method' => 'op_simple',
        'short' => t('='),
        'values' => 1,
      ),
      '!=' => array(
        'title' => t('Is not equal to'),
        'method' => 'op_simple',
        'short' => t('!='),
        'values' => 1,
      ),
      '>=' => array(
        'title' => t('Is greater than or equal to'),
        'method' => 'op_simple',
        'short' => t('>='),
        'values' => 1,
      ),
      '>' => array(
        'title' => t('Is greater than'),
        'method' => 'op_simple',
        'short' => t('>'),
        'values' => 1,
      ),
      'between' => array(
        'title' => t('Is between'),
        'method' => 'op_between',
        'short' => t('between'),
        'values' => 2,
      ),
      'not between' => array(
        'title' => t('Is not between'),
        'method' => 'op_between',
        'short' => t('not between'),
        'values' => 2,
      ),
    );

    // if the definition allows for the empty operator, add it.
    if (!empty($this->definition['allow empty'])) {
      $operators += array(
        'empty' => array(
          'title' => t('Is empty (NULL)'),
          'method' => 'op_empty',
          'short' => t('empty'),
          'values' => 0,
        ),
        'not empty' => array(
          'title' => t('Is not empty (NULL)'),
          'method' => 'op_empty',
          'short' => t('not empty'),
          'values' => 0,
        ),
      );
    }

    return $operators;
  }

  /**
   * Provide a list of all the numeric operators
   */
  function operator_options($which = 'title') {
    $options = array();
    foreach ($this->operators() as $id => $info) {
      $options[$id] = $info[$which];
    }

    return $options;
  }

  function operator_values($values = 1) {
    $options = array();
    foreach ($this->operators() as $id => $info) {
      if ($info['values'] == $values) {
        $options[] = $id;
      }
    }

    return $options;
  }
  /**
   * Provide a simple textfield for equality
   */
  function value_form(&$form, &$form_state) {
    $form['value']['#tree'] = TRUE;

    // We have to make some choices when creating this as an exposed
    // filter form. For example, if the operator is locked and thus
    // not rendered, we can't render dependencies; instead we only
    // render the form items we need.
    $which = 'all';
    if (!empty($form['operator'])) {
      $source = ($form['operator']['#type'] == 'radios') ? 'radio:options[operator]' : 'edit-options-operator';
    }

    if (!empty($form_state['exposed'])) {
      if (empty($this->options['expose']['operator'])) {
        // exposed and locked.
        $which = in_array($this->operator, $this->operator_values(2)) ? 'minmax' : 'value';
      }
      else {
        $source = 'edit-' . form_clean_id($this->options['expose']['operator']);
      }
    }

    if ($which == 'all' || $which == 'value') {
      $form['value']['value'] = array(
        '#type' => 'textfield',
        '#title' => t('Value'),
        '#size' => 30,
        '#default_value' => $this->value['value'],
      );
      if ($which == 'all') {
        $form['value']['value'] += array(
          '#process' => array('views_process_dependency'),
          '#dependency' => array($source => $this->operator_values(1)),
        );
      }
    }

    if ($which == 'all' || $which == 'minmax') {
      $form['value']['min'] = array(
        '#type' => 'textfield',
        '#title' => t('Min'),
        '#size' => 30,
        '#default_value' => $this->value['min'],
      );
      $form['value']['max'] = array(
        '#type' => 'textfield',
        '#title' => t('And max'),
        '#size' => 30,
        '#default_value' => $this->value['max'],
      );
      if ($which == 'all') {
        $dependency = array(
          '#process' => array('views_process_dependency'),
          '#dependency' => array($source => $this->operator_values(2)),
        );
        $form['value']['min'] += $dependency;
        $form['value']['max'] += $dependency;
      }
    }
  }

  function query() {
    $this->ensure_my_table();
    $field = "$this->table_alias.$this->real_field";

    $info = $this->operators();
    if (!empty($info[$this->operator]['method'])) {
      $this->{$info[$this->operator]['method']}($field);
    }
  }

  function op_between($field) {
    if ($this->operator == 'between') {
      $a = $this->value['min'];
      $b = $this->value['max'];
    }
    else {
      $a = $this->value['max'];
      $b = $this->value['min'];
    }

    $this->query->add_where($this->options['group'], "$field >= %d", $a);
    $this->query->add_where($this->options['group'], "$field <= %d", $b);
  }

  function op_simple($field) {
    $this->query->add_where($this->options['group'], "$field $this->operator %d", $this->value['value']);
  }

  function op_empty($field) {
    if ($this->operator == 'empty') {
      $operator = "IS NULL";
    }
    else {
      $operator = "IS NOT NULL";
    }

    $this->query->add_where($this->options['group'], "$field $operator");
  }

  function admin_summary() {
    $output = check_plain($this->operator) . ' ';
    if (in_array($this->operator, $this->operator_values(2))) {
      $output .= t('@min and @max', array('@min' => $this->value['min'], '@max' => $this->value['max']));
    }
    else {
      $output .= check_plain($this->value['value']);
    }
    return $output;
  }
}

class views_handler_filter_date extends views_handler_filter_numeric {
  function options(&$options) {
    parent::options($options);
    $options['value']['type'] = 'date';
  }

  /**
   * Add a type selector to the value form
   */
  function value_form(&$form, &$form_state) {
    $form['value']['type'] = array(
      '#type' => 'radios',
      '#title' => t('Value type'),
      '#options' => array(
        'date' => t('A date in any machine readable format. CCYY-MM-DD HH:MM:SS is preferred.'),
        'offset' => t('An offset from the current time such as "+1 day" or "-2 hours and 30 minutes"'),
      ),
      '#default_value' => $this->value['type'],
    );
    parent::value_form($form, $form_state);
  }

  function options_validate(&$form, &$form_state) {
    parent::options_validate($form, $form_state);
    $operators = $this->operators();
    if ($operators[$form_state['values']['options']['operator']]['values'] == 1) {
      $convert = strtotime($form_state['values']['options']['value']['value']);
      if ($convert == -1 || $convert === FALSE) {
        form_error($form['value']['value'], t('Invalid date format.'));
      }
    }
    else {
      $min = strtotime($form_state['values']['options']['value']['min']);
      if ($min == -1 || $min === FALSE) {
        form_error($form['value']['min'], t('Invalid date format.'));
      }
      $max = strtotime($form_state['values']['options']['value']['max']);
      if ($max == -1 || $max === FALSE) {
        form_error($form['value']['max'], t('Invalid date format.'));
      }
    }
  }

  function op_between($field) {
    if ($this->operator == 'between') {
      $a = strtotime($this->value['min'], 0);
      $b = strtotime($this->value['max'], 0);
    }
    else {
      $a = strtotime($this->value['max'], 0);
      $b = strtotime($this->value['min'], 0);
    }

    if ($this->value['type'] == 'offset') {
      $a = '***CURRENT_TIME***' . sprintf('%+d', $a); // keep sign
      $b = '***CURRENT_TIME***' . sprintf('%+d', $b); // keep sign
    }
    // %s is safe here because strtotime scrubbed the input and we might
    // have a string if using offset.
    $this->query->add_where($this->options['group'], "$field >= %s", $a);
    $this->query->add_where($this->options['group'], "$field <= %s", $b);
  }

  function op_simple($field) {
    $value = strtotime($this->value['value'], 0);
    if ($this->value['type'] == 'offset') {
      $value = '***CURRENT_TIME***' . sprintf('%+d', $value); // keep sign
    }
    $this->query->add_where($this->options['group'], "$field $this->operator %s", $value);
  }
}

/**
 * Complex filter to handle filtering for many to one relationships,
 * such as terms (many terms per node) or roles (many roles per user).
 *
 * The construct method needs to be overridden to provide a list of options;
 * alternately, the value_form and admin_summary methods need to be overriden
 * to provide something that isn't just a select list.
 */
class views_handler_filter_many_to_one extends views_handler_filter {
  function construct() {
    parent::construct();
    // @todo Pull depth information from definition.
    $this->allow_depth = FALSE;
    $this->value_title = t('Options');
    $this->value_options = array();
  }

  function options(&$options) {
    parent::options($options);
    $options['operator'] = 'or';
    $options['value'] = array();
    $options['depth'] = 0;
  }

  /**
   * Provide inclusive/exclusive matching
   */
  function operator_options() {
    return array(
      'or' => t('Is one of'),
      'and' => t('Is all of'),
      'not' => t('Is none of'),
    );
  }

  function value_form(&$form, &$form_state) {
    $form['value'] = array(
      '#type' => 'select',
      '#title' => $this->value_title,
      '#options' => $this->value_options,
      '#multiple' => TRUE,
      '#size' => count($this->value_options) > 8 ? 8 : $this->value_options,
      '#default_value' => (array) $this->value,
    );
  }

  function admin_summary() {
    if (!empty($this->options['exposed'])) {
      return t('exposed');
    }

    if (count($this->value) == 1) {
      // If there is only one, show it as an =.
      $keys = array_keys($this->value);
      $key = array_shift($keys);
      if (!empty($this->value_options[$key])) {
        $value = check_plain($this->value_options[$key]);
      }
      else {
        $value = t('Unknown');
      }

      return ($this->operator == 'in' ? '=' : '<>') . ' ' . $value;
    }
    $output = '';
    foreach ($this->value as $value) {
      if ($output) {
        $output .= ', ';
      }
      if (strlen($output) > 8) {
        $output .= '...';
        break;
      }
      $output .= check_plain($this->value_options[$value]);
    }

    return check_plain($this->operator) . ' ' . $output;
  }

  /**
   * Override ensure_my_table so we can control how this joins in.
   * The operator actually has influence over joining.
   */
  function ensure_my_table() {
    if (!isset($this->table_alias)) {
      $base_join = views_get_table_join($this->table, $this->query->primary_table);
      if ($this->operator != 'not') {
        // If it's an and or an or, we do one join per selected value.
        // Clone the join for each table:
        $this->table_aliases = array();
        foreach ($this->value as $value) {
          // Clone to make sure we aren't overwriting previous joins due
          // to overzealous references.
          $join = drupal_clone($base_join);
          $join->extra = array(
            array(
              'field' => $this->real_field,
              'value' => $value,
              'numeric' => !empty($this->definition['numeric']),
            ),
          );
          $alias = $this->table_aliases[$value] = $this->query->add_table($this->table, $this->relationship, $join, $this->table . '_' . $value);

          // and set table_alias to the first of these.
          if (empty($this->table_alias)) {
            $this->table_alias = $alias;
          }
        }
      }
      else {
        // For not, we just do one join. We'll add a where clause during
        // the query phase to ensure that $table.$field IS NULL.
        $join = drupal_clone($base_join); // be safe!
        $join->type = 'LEFT';
        $join->extra = array();
        $join->extra_type = 'OR';
        foreach ($this->value as $value) {
          $join->extra[] = array(
            'field' => $this->field,
            'value' => $value,
            'numeric' => !empty($this->definition['numeric']),
          );
        }

        $this->table_alias = $this->query->add_table($this->table, $this->relationship, $join);
      }
    }
    return $this->table_alias;
  }

  function query() {
    if (empty($this->value)) {
      return;
    }
    $this->ensure_my_table();

    if ($this->operator == 'not') {
      $this->query->add_where($this->options['group'], "$this->table_alias.$this->real_field IS NULL");
    }
    else {
      $clauses = array();
      foreach ($this->table_aliases as $value => $alias) {
        if (!empty($this->definition['numeric'])) {
          $clauses[] = "$alias.$this->real_field = %d";
        }
        else {
          $clauses[] = "$alias.$this->real_field = '%s'";
        }
      }

      // implode on either AND or OR.
      $this->query->add_where($this->options['group'], implode(' ' . strtoupper($this->operator) . ' ', $clauses), $this->value);
    }
  }
}

/**
 * @}
 */

/**
 * @defgroup views_argument_handlers Handlers for arguments
 * @{
 */
/**
 * Base class for arguments.
 *
 * The basic argument works for very simple arguments such as nid and uid
 */
class views_handler_argument extends views_handler {
  var $name_field = NULL;
  /**
   * Constructor
   */
  function construct() {
    if (!empty($this->definition['name field'])) {
      $this->name_field = $this->definition['name field'];
    }
  }

  /**
   * Give an argument the opportunity to modify the breadcrumb, if it wants.
   * This only gets called on displays where a breadcrumb is actually used.
   *
   * The breadcrumb will be in the form of an array, with the keys being
   * the path and the value being the already sanitized title of the path.
   */
  function set_breadcrumb(&$breadcrumb) { }

  /**
   * Provide defaults for the argument when a new one is created.
   */
  function options(&$options) {
    parent::options($options);
    $options['default_action'] = 'ignore';
    $options['style_plugin'] = 'default_summary';
    $options['style_options'] = array();
    $options['wildcard'] = 'all';
    $options['wildcard_substitution'] = t('All');
    $options['title'] = '';
  }

  /**
   * Provide a default options form for the argument.
   */
  function options_form(&$form, &$form_state) {
    $defaults = $this->default_actions();
    foreach ($defaults as $id => $info) {
      $options[$id] = $info['title'];
    }
    $form['default_action'] = array(
      '#prefix' => '<div class="views-left-50">',
      '#suffix' => '</div>',
      '#type' => 'radios',
      '#title' => t('Action to take if argument is not present'),
      '#options' => $options,
      '#default_value' => $this->options['default_action'],
    );
    $form['wildcard'] = array(
      '#prefix' => '<div class="views-left-40">',
      // prefix and no suffix means these two items will be grouped together.
      '#type' => 'textfield',
      '#title' => t('Wildcard'),
      '#size' => 20,
      '#default_value' => $this->options['wildcard'],
      '#description' => t('If this value is received as an argument, the argument will be ignored; i.e, "all values"'),
    );
    $form['wildcard_substitution'] = array(
      '#suffix' => '</div>',
      '#type' => 'textfield',
      '#title' => t('Wildcard title'),
      '#size' => 20,
      '#default_value' => $this->options['wildcard_substitution'],
      '#description' => t('The title to use for the wildcard in substitutions elsewhere.'),
    );
    $form['title'] = array(
      '#prefix' => '<div class="clear">',
      '#suffix' => '</div>',
      '#type' => 'textfield',
      '#title' => t('Title'),
      '#default_value' => $this->options['title'],
      '#description' => t('The title to use when this argument is present; it will override the title of the view and titles from previous arguments.'),
    );
  }

  /**
   * Provide a list of default behaviors for this argument if the argument
   * is not present.
   *
   * Override this method to provide additional (or fewer) default behaviors.
   */
  function default_actions($which = NULL) {
    $defaults = array(
      'ignore' => array(
        'title' => t('Display all values'),
        'method' => 'default_ignore',
        'breadcrumb' => TRUE, // generate a breadcrumb to here
      ),
      'not found' => array(
        'title' => t('Display page not found'),
        'method' => 'default_not_found',
      ),
      'empty' => array(
        'title' => t('Display empty text'),
        'method' => 'default_empty',
        'breadcrumb' => TRUE, // generate a breadcrumb to here
      ),
      'summary asc' => array(
        'title' => t('Summary, sorted ascending'),
        'method' => 'default_summary',
        'method args' => array('asc'),
        'style plugin' => TRUE,
        'breadcrumb' => TRUE, // generate a breadcrumb to here
      ),
      'summary desc' => array(
        'title' => t('Summary, sorted descending'),
        'method' => 'default_summary',
        'method args' => array('desc'),
        'style plugin' => TRUE,
        'breadcrumb' => TRUE, // generate a breadcrumb to here
      ),
    );

    if ($which) {
      if (!empty($defaults[$which])) {
        return $defaults[$which];
      }
    }
    else {
      return $defaults;
    }
  }

  /**
   * Determine if the can generate a breadcrumb
   *
   * @return TRUE/FALSE
   */
  function uses_breadcrumb() {
    $info = $this->default_actions($this->options['default_action']);
    return !empty($info['breadcrumb']);
  }

  /**
   * Determine if the argument needs a style plugin.
   *
   * @return TRUE/FALSE
   */
  function needs_style_plugin() {
    $info = $this->default_actions($this->options['default_action']);
    return !empty($info['style plugin']);
  }

  /**
   * Handle the default action, which means our argument wasn't present.
   *
   * Override this method only with extreme care.
   *
   * @return
   *   A boolean value; if TRUE, continue building this view. If FALSE,
   *   building the view will be aborted here.
   */
  function default_action() {
    $info = $this->default_actions($this->options['default_action']);
    if (!$info) {
      return FALSE;
    }

    if (!empty($info['method args'])) {
      return call_user_func_array(array($this, $info['method']), $info['method args']);
    }
    else {
      return $this->{$info['method']}();
    }
  }

  /**
   * Default action: ignore.
   *
   * If an argument was expected and was not given, in this case, simply
   * ignore the argument entirely.
   */
  function default_ignore() {
    return TRUE;
  }

  /**
   * Default action: not found.
   *
   * If an argument was expected and was not given, in this case, report
   * the view as 'not found' or hide it.
   */
  function default_not_found() {
    // Set a failure condition and let the display manager handle it.
    $this->view->build_info['fail'] = TRUE;
    return FALSE;
  }

  /**
   * Default action: empty
   *
   * If an argument was expected and was not given, in this case, display
   * the view's empty text
   */
  function default_empty() {
    // We return with no query; this will force the empty text.
    $this->view->built = TRUE;
    $this->view->executed = TRUE;
    $this->view->result = array();
    return FALSE;
  }

  /**
   * Default action: summary.
   *
   * If an argument was expected and was not given, in this case, display
   * a summary query.
   */
  function default_summary($order) {
    $this->view->build_info['summary'] = TRUE;
    $this->view->build_info['summary_level'] = $this->options['id'];

    // Change the display style to the summary style for this
    // argument.
    $this->view->style_plugin = $this->options['style_plugin'];
    $this->view->style_options = $this->options['style_options'];

    // Clear out the normal primary field and whatever else may have
    // been added and let the summary do the work.
    $this->query->clear_fields();
    $this->summary_query();

    $this->summary_sort($order);

    // Summaries have their own sorting and fields, so tell the View not
    // to build these.
    $this->view->build_sort = $this->view->build_fields = FALSE;
    return TRUE;
  }

  /**
   * Build the info for the summary query.
   *
   * This must:
   * - add_groupby: group on this field in order to create summaries.
   * - add_field: add a 'num_nodes' field for the count. Usually it will
   *   be a count on $view->primary_field
   * - set_count_field: Reset the count field so we get the right paging.
   *
   * @return
   *   The alias used to get the number of records (count) for this entry.
   */
  function summary_query() {
    $this->ensure_my_table();
    // Add the field.
    $this->base_alias = $this->query->add_field($this->table_alias, $this->real_field);

    // Add the 'name' field. For example, if this is a uid argument, the
    // name field would be 'name' (i.e, the username).
    if (isset($this->name_field)) {
      $this->name_alias = $this->query->add_field($this->table_alias, $this->name_field);
    }
    else {
      $this->name_alias = $this->base_alias;
    }

    return $this->summary_basics();
  }

  /**
   * Some basic summary behavior that doesn't need to be repeated as much as
   * code that goes into summary_query()
   */
  function summary_basics($count_field = TRUE) {
    // Add the number of nodes counter
    $count_alias = $this->query->add_field(NULL, 'COUNT('. $this->query->primary_table .'.'. $this->query->primary_field . ')', 'num_records');
    $this->query->add_groupby($this->base_alias);

    if ($count_field) {
      $this->query->set_count_field($this->table_alias, $this->real_field);
    }

    $this->count_alias = $count_alias;
  }

  /**
   * Sorts the summary based upon the user's selection. The base variant of
   * this is usually adequte.
   *
   * @param $order
   *   The order selected in the UI.
   */
  function summary_sort($order) {
    $this->query->add_orderby(NULL, NULL, $order, $this->base_alias);
  }

  /**
   * Provides a link from the summary to the next level; this will be called
   * once per row of a summary.
   *
   * @param $data
   *   The query results for the row.
   * @param $url
   *   The base URL to use.
   */
  function summary_link($data, $url) {
    $value = $data->{$this->base_alias};
    return url("$url/$value");
  }

  /**
   * Provides the name to use for the summary. By default this is just
   * the name field.
   *
   * @param $data
   *   The query results for the row.
   */
  function summary_name($data) {
    return check_plain($data->{$this->name_alias});
  }

  /**
   * Set up the query for this argument.
   *
   * The argument sent may be found at $this->argument.
   */
  function query() {
    $this->ensure_my_table();
    $this->query->add_where(0, "$this->table_alias.$this->real_field = '%s'", $this->argument);
  }

  /**
   * Get the title this argument will assign the view, given the argument.
   *
   * This usually needs to be overridden to provide a proper title.
   */
  function title() {
    return check_plain($this->argument);
  }

  /**
   * Validate that this argument works. By default, all arguments are valid.
   */
  function validate($arg) {
    return TRUE;
  }
}

/**
 * Abstract argument handler for simple formulae.
 *
 * Child classes of this object should implement summary_link, at least.
 */
class views_handler_argument_formula extends views_handler_argument {
  var $formula = NULL;
  /**
   * Constructor
   */
  function construct() {
    if (!empty($this->definition['formula'])) {
      $this->formula = $this->definition['formula'];
    }
  }

  function get_formula() {
    return str_replace('***table***', $this->table_alias, $this->formula);
  }

  /**
   * Build the summary query based on a formula
   */
  function summary_query() {
    $this->ensure_my_table();
    // Now that our table is secure, get our formula.
    $formula = $this->get_formula();

    // Add the field.
    $this->base_alias = $this->name_alias = $this->query->add_field(NULL, $formula, $this->field);
    $this->query->set_count_field(NULL, $formula, $this->field);

    return $this->summary_basics(FALSE);
  }

  /**
   * Build the query based upon the formula
   */
  function query() {
    $this->ensure_my_table();
    // Now that our table is secure, get our formula.
    $formula = $this->get_formula();

    $this->query->add_where(0, "$formula = '%s'", $this->argument);
  }
}

/**
 * Basic argument handler to implement string arguments that may have length
 * limits.
 */
class views_handler_argument_string extends views_handler_argument {
  function options(&$options) {
    parent::options($options);
    $options['glossary'] = FALSE;
    $options['limit'] = 0;
    $options['case'] = 'none';
    $options['path_case'] = 'none';
    $options['transform_dash'] = FALSE;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['glossary'] = array(
      '#type' => 'checkbox',
      '#title' => t('Glossary mode'),
      '#description' => t('Glossary mode applies a limit to the number of characters used in the argument, which allows the summary view to act as a glossary.'),
      '#default_value' => $this->options['glossary'],
    );

    $form['limit'] = array(
      '#type' => 'textfield',
      '#title' => t('Character limit'),
      '#description' => t('How many characters of the argument to filter against. If set to 1, all fields starting with the letter in the argument would be matched.'),
      '#default_value' => $this->options['limit'],
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-glossary' => array(TRUE)),
    );

    $form['case'] = array(
      '#type' => 'select',
      '#title' => t('Case'),
      '#description' => t('When printing the argument result, how to transform the case.'),
      '#options' => array(
        'none' => t('No transform'),
        'upper' => t('Upper case'),
        'lower' => t('Lower case'),
        'ucfirst' => t('Capitalize first letter'),
        'ucwords' => t('Capitalize each word'),
      ),
      '#default_value' => $this->options['case'],
    );

    $form['path_case'] = array(
      '#type' => 'select',
      '#title' => t('Case in path'),
      '#description' => t('When printing url paths, how to transform the of the argument. Do not use this unless with Postgres as it uses case sensitive comparisons.'),
      '#options' => array(
        'none' => t('No transform'),
        'upper' => t('Upper case'),
        'lower' => t('Lower case'),
        'ucfirst' => t('Capitalize first letter'),
        'ucwords' => t('Capitalize each word'),
      ),
      '#default_value' => $this->options['path_case'],
    );

    $form['transform_dash'] = array(
      '#type' => 'checkbox',
      '#title' => t('Transform spaces to dashes in URL'),
      '#description' => t('Glossary mode applies a limit to the number of characters used in the argument, which allows the summary view to act as a glossary.'),
      '#default_value' => $this->options['transform_dash'],
    );
  }

  /**
   * Build the summary query based on a formula
   */
  function summary_query() {
    $this->ensure_my_table();
    if (empty($this->options['glossary'])) {
      // Add the field.
      $this->base_alias = $this->name_alias = $this->query->add_field($this->table_alias, $this->real_field);
      $this->query->set_count_field($this->table_alias, $this->real_field);
    }
    else {
      // Add the field.
      $formula = $this->get_formula();
      $this->base_alias = $this->name_alias = $this->query->add_field(NULL, $formula, $this->field . '_truncated');
      $this->query->set_count_field(NULL, $formula, $this->field, $this->field . '_truncated');
    }

    return $this->summary_basics(FALSE);
  }

  /**
   * Get the formula for this argument.
   *
   * $this->ensure_my_table() MUST have been called prior to this.
   */
  function get_formula() {
    return "LEFT($this->table_alias.$this->real_field, " . intval($this->options['limit']) . ")";
  }

  /**
   * Build the query based upon the formula
   */
  function query() {
    $this->ensure_my_table();

    $argument = $this->argument;
    if (!empty($this->options['transform_dash'])) {
      $argument = strtr($argument, '-', ' ');
    }

    if (empty($this->options['glossary'])) {
      $field = "$this->table_alias.$this->real_field";
    }
    else {
      $field = $this->get_formula();
    }

    $this->query->add_where(0, "$field = '%s'", $argument);
  }

  /**
   * Provides a link from the summary to the next level; this will be called
   * once per row of a summary.
   *
   * @param $data
   *   The query results for the row.
   * @param $url
   *   The base URL to use.
   */
  function summary_link($data, $url) {
    $value = $this->case_transform($data->{$this->base_alias}, 'path_case');
    if (!empty($this->options['transform_dash'])) {
      $value = strtr($value, ' ', '-');
    }
    return url("$url/$value");
  }

  function case_transform($string, $option) {
    switch ($this->options[$option]) {
      default:
        return $string;
      case 'upper':
        return strtoupper($string);
      case 'lower':
        return strtolower($string);
      case 'upper':
        return strtoupper($string);
      case 'ucfirst':
        return ucfirst($string);
      case 'ucwords':
        return ucwords($string);
    }
  }

  function title() {
    return check_plain($this->case_transform($this->argument, 'case'));
  }
}

/**
 * @}
 */
