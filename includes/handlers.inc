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
//  dpr('Instantiating handler ' . $definition['handler']);
  $handler = new $definition['handler'];
  if (isset($definition['arguments'])) {
    call_user_func_array(array($handler, 'construct'), $definition['arguments']);
  }

  $handler->set_definition($definition);
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
    $handler = new $h['handler'];
    if (isset($h['arguments'])) {
      call_user_func_array(array($handler, 'construct'), $h['arguments']);
    }
    return $handler;
  }
  // DEBUG -- identify missing handlers
  dsm("missing join: $table $primary_table");
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
 *   function construct($left_table, $left_field, $field, $extra = array(), $type = 'LEFT') {
 *     parent::construct($left_table, $left_field, $field, $extra, $type);
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
 */
class views_join {
  /**
   * Construct the views_join object.
   */
  function construct($table, $left_table, $left_field, $field, $extra = array(), $type = 'LEFT') {
    $this->table = $table;
    $this->left_table = $left_table;
    $this->left_field = $left_field;
    $this->field = $field;
    $this->extra = $extra;
    $this->type = strtoupper($type);
  }

  /**
   * Build the SQL for the join this object represents.
   */
  function join($table, &$query) {
    $left = $query->get_table_info($this->left_table);
    $output = " $this->type JOIN {" . $this->table . "} $table[alias] ON $left[alias].$this->left_field = $table[alias].$this->field";

    // Tack on the extra.
    if (isset($extra)) {
      foreach ($extra as $field => $value) {
        $output .= " AND $table[alias].$this->field";
        if (is_array($value) && !empty($value)) {
          $output .= " IN ('". implode("','", $value) ."')";
        }
        else if ($value !== NULL) {
          $output .= " = '$value'";
        }
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
   * init the handler with necessary data.
   * @param $view
   *   The $view object this handler is attached to.
   * @param $data
   *   The item from the database; the actual contents of this will vary
   *   based upon the type of handler.
   */
  function init(&$view, &$data) {
    $this->view = &$view;
    $this->data = &$data;

    // Mostly this exists to make things easier to reference. $this->options['...']
    // is a little easier than $this->data->options['...'];
    if (isset($data->options)) {
      $this->options = $data->options;
    }
    else {
      $this->options = array();
    }

    // This exist on most handlers, but not all. So they are still optional.
    if (isset($data->tablename)) {
      $this->table = $data->tablename;
    }

    if (isset($data->field)) {
      $this->field = $data->field;
      if (!isset($this->real_field)) {
        $this->real_field = $data->field;
      }
    }

    if (isset($data->relationship)) {
      $this->relationship = $data->relationship;
    }

    if (!empty($view->query)) {
      $this->query = &$view->query;
    }
  }

  /**
   * Provide a form for setting options.
   */
  function options_form(&$form) { }

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
    if (!isset($this->alias)) {
      $this->table_alias = $this->query->ensure_table($this->table, $this->relationship);
    }
    return $this->table_alias;
  }
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
   * Called to implement a relationship in a query.
   */
  function query() {
    $alias = $this->table . '_' . $this->field . '_' . $this->relationship;
    return $this->query->add_relationship($alias, new views_join($this->view->primary_table, $this->table, $this->real_field, $this->primary_field), $this->relationship);
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
  function construct($click_sortable = FALSE, $additional_fields = array()) {
    $this->click_sortable = $click_sortable;
    $this->additional_fields = $additional_fields;
  }

  /**
   * Called to add the field to a query.
   */
  function query() {
    $this->ensure_my_table();
    // Add the field.
    $this->field_alias = $this->query->add_field($this->table_alias, $this->real_field);

    // Add any additional fields we are given.
    if (!empty($this->additional_fields) && is_array($this->additional_fields)) {
      foreach ($this->additional_fields as $field) {
        $this->aliases[$field] = $this->query->add_field($this->table_alias, $field);
      }
    }
  }

  /**
   * Called to determine what to tell the clicksorter.
   */
  function click_sort() {
    return "$this->field_alias";
  }

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
   * Constructor; calls to base object constructor.
   */
  function construct($click_sortable = FALSE, $additional_fields = array()) {
    parent::construct($click_sortable, $additional_fields);
  }

  function options_form(&$form) {
    $form['date_format'] = array(
      '#type' => 'select',
      '#title' => t('Date format'),
      '#options' => array(
        'small' => t('Small'),
        'medium' => t('Medium'),
        'large' => t('Large'),
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
    );
  }

  function render($values) {
    $value = $values->{$this->field_alias};
    $format = !empty($this->options['date_format']) ? $this->options['date_format'] : 'medium';
    if ($format == 'custom') {
      $custom_format = $this->options['custom_date_format'];
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
  /**
   * Constructor; calls to base object constructor.
   */
  function construct($click_sortable = FALSE, $additional_fields = array()) {
    parent::construct($click_sortable, $additional_fields);
  }

  function options_form(&$form) {
    $form['type'] = array(
      '#type' => 'select',
      '#title' => t('Date format'),
      '#options' => array(
        'yes-no' => t('Yes/No'),
        'true-false' => t('True/False'),
        'on-off' => t('On/Off'),
      ),
      '#default_value' => isset($this->options['type']) ? $this->options['type'] : 'yes-no',
    );
    $form['not'] = array(
      '#type' => 'checkbox',
      '#title' => t('Reverse'),
      '#description' => t('If checked, true will be displayed as false.'),
      '#default_value' => !empty($this->options['not']),
    );
  }

  function render($values) {
    $value = $values->{$this->field_alias};
    if (!empty($this->options['not'])) {
      $value = !$value;
    }

    $format = isset($this->options['type']) ? $this->options['type'] : 'yes';

    switch ($format) {
      case 'yes-no':
      default:
        return $value ? t('Yes') : t('No');
      case 'true-false':
        return $value ? t('True') : t('False');
      case 'yes-no':
        return $value ? t('On') : t('Off');
    }
  }
}

/**
 * A handler to run a field through check_markup, using a companion
 * format field.
 *
 * Allows for display of true/false, yes/no, on/off.
 */
class views_handler_field_markup extends views_handler_field {
  /**
   * Constructor; calls to base object constructor.
   */
  function construct($format) {
    $this->format = $format;

    $addl_fields = array();
    if (!is_numeric($format)) {
      $addl_fields[] = $format;
    }

    parent::construct(FALSE, $addl_fields);
  }

  function render($values) {
    $value = $values->{$this->field_alias};
    $format = is_numeric($this->format) ? $this->format : $values->{$this->aliases[$this->format]};
    return check_markup($value, $format, FALSE);
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
    $this->query->add_orderby($this->table_alias, $this->real_field, $this->data->order);
  }
}

/**
 * Base sort handler that has no options and performs a simple sort
 */
class views_handler_sort_formula extends views_handler_sort {
  /**
   * Constructor to take the formula this sorts on.
   *
   * @param $formula
   *   The formula used to sort. If an array, may be keyed by database type. If
   *   used, 'default' MUST be defined.
   */
  function construct($formula) {
    $this->formula = $formula;
    if (is_array($formula) && !isset($formula['default'])) {
      $this->error = t('views_handler_sort_formula missing default: @formula', array('@formula' => var_export($formula, TRUE)));
    }
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
    $this->add_orderby(NULL, $this->formula, $this->data->order, $this->table_alias . '_' . $this->field);
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
   * Provide a form for setting the operator.
   */
  function operator_form(&$form) { }

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
   * Provide a form for setting options.
   */
  function value_form(&$form) { }

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
   * Add this filter to the query.
   */
  function query() {
    $this->ensure_my_table();
    $this->query->add_where($this->data->group, "$this->table_alias.$this->real_field " . $this->data->operator . " '%s'", $this->data->value);
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
  function construct($name_field = NULL) {
    $this->name_field = $name_field;
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
    $count_alias = $this->query->add_field(NULL, 'COUNT(' . $this->query->primary_field . ')', 'num_records');
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
    $query->add_orderby(NULL, NULL, $order, $this->base_alias);
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
   * Provide a list of default behaviors for this argument if the argument
   * is not present.
   *
   * Override this method to provide additional (or fewer) default behaviors.
   */
  function defaults() {
    return array(
      'ignore' => t('Display all values'),
      'not found' => t('Display page not found'),
      'empty' => t('Display empty text'),
      'summary asc' => t('Summary, sorted ascending'),
      'summary desc' => t('Summary, sorted descending'),
    );
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
    $action = $this->data->default_action;
    switch ($action) {
      default:
      case 'ignore':
        // Do nothing at all.
        return TRUE;
      case 'not found':
        // Set a failure condition and let the display manager handle it.
        $this->view->build_info['fail'] = TRUE;
        return FALSE;
      case 'empty':
        // We return with no query; this will force the empty text.
        $this->view->built = TRUE;
        return FALSE;
      case 'summary':
      case 'summary asc':
      case 'summary desc':
        $this->view->build_info['summary'] = TRUE;
        $this->view->build_info['summary_level'] = $this->data->position;

        // Change the display style to the summary style for this
        // argument.
        $this->view->style_plugin = isset($this->options['style_plugin']) ? $this->options['style_plugin'] : 'default_summary';

        // Give it the style's options, too.
        // @todo


        // Clear out the normal primary field and whatever else may have
        // been added and let the summary do the work.
        $this->query->clear_fields();
        $this->summary_query();

        // Allow the new style to add additional fields if it wants.
        // @todo

        // Cut 'summary' out of our action to see how, if at all, we should
        // sort.
        $order = trim(str_replace($action, 'summary', ''));
        if ($order) {
          $argument->handler->summary_sort($order);
        }

        // DISTINCT can cause the summaries to fail.
        // @todo: This may not be true anymore.
//        $this->query->no_distinct = TRUE;


        // Summaries have their own sorting and fields, so tell the View not
        // to build these.
        $this->view->build_sort = $this->view->build_fields = FALSE;
        return TRUE;
    }
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
  /**
   * Constructor
   */
  function construct($formula) {
    $this->formula = $formula;
  }

  /**
   * Build the summary query based on a formula
   */
  function summary_query() {
    $this->ensure_my_table();
    $field_alias = $alias . '_' . $this->field;
    // Add the field.
    $this->base_alias = $this->query->add_field(NULL, $this->formula, $field_alias);
    $this->query->set_count_field(NULL, $this->formula, $field_alias);

    return $this->summary_basics(FALSE);
  }

  /**
   * Build the query based upon the formula
   */
  function query() {
    $this->ensure_my_table();
    $field_alias = $alias . '_' . $this->field;
    // Add the field.
    $this->name_alias = $this->query->add_field(NULL, $this->formula, $field_alias);
    $this->query->add_where(0, "$field = '%s'", $this->argument);
  }
}

/**
 * Argument handler for a year (CCYY)
 */
class views_handler_argument_date_year extends views_handler_argument_formula {
  /**
   * Constructor implementation
   */
  function construct() {
    $timezone = views_get_timezone();
    $this->formula = "YEAR(FROM_UNIXTIME(node.created+$timezone))";
  }

  /**
   * Provide a link to the next level of the view
   */
  function summary_link($data, $url) {
    $value = $data->{$this->base_alias};
    return l($value, "$url/$value");
  }
}

/**
 * Argument handler for a year plus month (CCYYMM)
 */
class views_handler_argument_date_year_month extends views_handler_argument_formula {
  /**
   * Constructor implementation
   */
  function construct() {
    $timezone = views_get_timezone();
    $this->formula =  "DATE_FORMAT(FROM_UNIXTIME(node.created+$timezone), '%Y%m')";
    $this->format = 'F, Y';
  }

  /**
   * Provide a link to the next level of the view
   */
  function summary_link($data, $url) {
    $value = $data->{$this->base_alias};
    $created = $data->{$this->name_alias};
    return l(format_date($created, 'custom', $this->format), "$url/$value");
  }

  /**
   * Provide a link to the next level of the view
   */
  function title($data, $url) {
    return format_date(strtotime($this->argument . "15"), 'custom', $this->format, 0);
  }
}

/**
 * Argument handler for a month (MM)
 */
class views_handler_argument_date_month extends views_handler_argument_formula {
  /**
   * Constructor implementation
   */
  function construct() {
    $timezone = views_get_timezone();
    $this->formula =  "MONTH(FROM_UNIXTIME(node.created+$timezone))";
    $this->format = 'F';
  }

  /**
   * Provide a link to the next level of the view
   */
  function summary_link($data, $url) {
    $value = $data->{$this->base_alias};
    $created = $data->{$this->name_alias};
    return l(format_date($created, 'custom', $this->format), "$url/$value");
  }

  /**
   * Provide a link to the next level of the view
   */
  function title($data, $url) {
    return format_date(strtotime("2005" . $this->argument . "15"), 'custom', $this->format, 0);
  }
}

/**
 * @}
 */
