<?php
// $Id$
/**
 * @file
 * Views' argument handlers.
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
      // @todo: There should be a 'default' where a default value
      // is filled in if the argument is not present.
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
