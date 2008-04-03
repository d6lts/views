<?php
// $Id$
/**
 * @file
 * Views' filter handlers.
 */
/**
 * @defgroup views_sort_handlers Views' sort handlers
 * @{
 * Handlers to tell Views how to sort queries
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
    return (!empty($this->options['exposed']) ? t('exposed>') : '') . $options[$this->operator];
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
    if (!empty($form['operator'])) {
      $source = ($form['operator']['#type'] == 'radios') ? 'radio:options[operator]' : 'edit-options-operator';
    }
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
    $this->value_options = NULL;
  }

  /**
   * Child classes should override this function to set the 'value options'.
   * This can use a guard to be used to reduce database hits as much as
   * possible.
   */
  function get_value_options() {
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
    $this->get_value_options();
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

    $this->get_value_options();
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
    $placeholder = !empty($this->definition['numeric']) ? '%d' : "'%s'";

    $replace = array_fill(0, sizeof($this->value), $placeholder);
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
 *
 * @todo Should this handler allow an option as to whether or not to avoid
 * duplicates? That would have an effect on the join, but if you're not
 * concerned about duplicates you'll get more efficient queries.
 */
class views_handler_filter_many_to_one extends views_handler_filter_in_operator {
  function options(&$options) {
    parent::options($options);
    $options['operator'] = 'or';
    $options['value'] = array();
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
    $this->get_value_options();
    $form['value'] = array(
      '#type' => 'select',
      '#title' => $this->value_title,
      '#options' => $this->value_options,
      '#multiple' => TRUE,
      '#size' => count($this->value_options) > 8 ? 8 : $this->value_options,
      '#default_value' => (array) $this->value,
    );
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


