<?php
// $Id$
/**
 * @file handlers.inc
 * Defines the various handler objects to help build and display views.
 */

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
 *   function views_complex_join($left_table, $left_field, $field, $extra = array(), $type = 'LEFT') {
 *     parent::views_join($left_table, $left_field, $field, $extra, $type);
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
  function views_join($table, $left_table, $left_field, $field, $extra = array(), $type = 'LEFT') {
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
class views_handler {
  /**
   * Seed the handler with necessary data.
   * @param $view
   *   The $view object this handler is attached to.
   * @param $data
   *   The item from the database; the actual contents of this will vary
   *   based upon the type of handler.
   */
  function seed(&$view, &$data) {
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
    if (isset($data->table)) {
      $this->table = $data->table;
    }

    if (isset($data->field)) {
      $this->field = $data->field;
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
    return $this->query->add_relationship($alias, new views_join($this->view->primary_table, $this->table, $this->field, $this->primary_field), $this->relationship);
  }
}

/**
 * @}
 */

/**
 * @defgroup views_field_handlers Views' field handlers
 * @{
 * Handlers to tell Views how to build and display fields.
 */

/**
 * Base field handler that has no options and renders an unformatted field.
 */
class views_handler_field extends views_handler {

  /**
   * Construct a new field handler.
   */
  function views_handler_field($click_sortable = FALSE, $additional_fields = array()) {
    $this->click_sortable = $click_sortable;
    $this->additional_fields = $additional_fields;
  }

  /**
   * Called to add the field to a query.
   */
  function query() {
    // Ensure the requested table is part of the query, and get the proper alias fori t.
    $alias = $this->query->ensure_table($this->table, $this->relationship);
    // Add the field.
    $this->field_alias = $this->query->add_field($alias, $this->field);
    // Add any additional fields we are given.
    if (!empty($this->additional_fields) && is_array($this->additional_fields)) {
      foreach ($this->additional_fields as $this->field) {
        $this->query->add_field($alias, $this->field);
      }
    }
  }

  /**
   * Called to determine what to tell the clicksorter.
   */
  function click_sort() {
    // Ensure the requested table is part of the query, and get the proper alias for it.
    $alias = $this->query->ensure_table($this->table, $this->relationship);
    return "$alias.$this->field";
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
  function views_handler_field_date($click_sortable = FALSE, $additional_fields = array()) {
    parent::views_handler_field($click_sortable, $additional_fields);
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
    $format = $this->options['date_format'];
    $custom_format = $this->options['custom_date_format'];

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
    // Ensure the requested table is part of the query, and get the proper alias for it.
    $alias = $this->query->ensure_table($this->table, $this->relationship);
    // Add the field.
    $this->query->add_orderby($alias, $this->field, $this->data->order);
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
  function views_handler_sort_formula($formula) {
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
    // Ensure the requested table is part of the query, and get the proper alias for it.
    $alias = $this->query->ensure_table($this->table, $this->relationship);
    // Add the field.
    $this->add_orderby(NULL, $this->formula, $this->data->order, $alias . '_' . $this->field);
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
    $alias = $this->query->ensure_table($this->table, $this->relationship);
    $this->query->add_where($this->data->group, "$alias.$this->field " . $this->data->operator . " '%s'", $this->data->value);
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
  /**
   * Constructor
   */
  function views_handler_argument($name_field = NULL) {
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
    $alias = $this->query->ensure_table($this->table, $this->relationship);
    // Add the field.
    $this->base_alias = $this->query->add_field($alias, $this->field);    
    
    // Add the 'name' field. For example, if this is a uid argument, the
    // name field would be 'name' (i.e, the username).
    if (isset($this->name_field)) {
      $this->name_alias = $this->query->add_field($alias, $this->name_field);
    }
    else {
      $this->name_alias = $this->base_alias;
    }

    // Add the number of nodes counter
    $count_alias = $this->query->add_field(NULL, 'COUNT(' . $this->query->primary_field . ')', 'num_records');
    $this->query->add_groupby($this->base_alias);

    $this->query->set_count_field($alias, $this->field);

    return $count_alias;
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
    return l($data->{$this->name_alias}, "$url/$this->base_field");
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

        // Clear out the normal primary field and whatever else may have
        // been added and let the summary do the work.
        $this->query->clear_fields();
        $this->summary_query();

        // Cut 'summary' out of our action to see how, if at all, we should
        // sort.
        $order = trim(str_replace($action, 'summary', ''));
        if ($order) {
          $argument->handler->summary_sort($order);
        }

        // DISTINCT can cause the summaries to fail.
        // TODO: This may not be true anymore.
//        $this->query->no_distinct = TRUE;

        // Summaries have their own sorting and fields, so tell the View not
        // to build these.
        $this->view->build_sort = $this->view->build_fields = FALSE;
    }
  }

  /**
   * Set up the query for this argument.
   *
   * The argument sent may be found at $this->argument.
   */
  function query() {
    // Ensure the requested table is part of the query, and get the proper alias fori t.
    $alias = $this->query->ensure_table($this->table, $this->relationship);
    // Add the field.
    $field = $this->query->add_field($alias, $this->field);
    $this->query->add_where(0, "$field = '%s'", $this->argument);
  }

  /**
   * Get the title this argument will assign the view, given the argument.
   *
   * This usually needs to be overridden to provide a proper title.
   */
  function title($argument) {
    return check_plain($argument);
  }
}

/**
 * Argument handler for simple formulae.
 */
class views_handler_argument_date extends views_handler_argument {
  
}

/**
 * @}
 */
