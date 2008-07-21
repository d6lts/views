<?php
// $Id$
/**
 * @file
 * Views' filter handlers.
 */

/**
 * @defgroup views_filter_handlers Views' filter handlers
 * @{
 * Handlers to tell Views how to filter queries.
 *
 * Definition items:
 * - allow empty: If true, the 'IS NULL' and 'IS NOT NULL' operators become
 *   available as standard operators.
 * -
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

    $this->operator = $this->options['operator'];
    $this->value = $this->options['value'];

    // Compatibility: Set use_operator to true if the old way of using
    // the operator is set and use_operator is NULL (was never set).
    if (!empty($options['exposed']) && !empty($options['expose']['operator']) && !isset($options['expose']['use_operator'])) {
      $this->options['expose']['use_operator'] = TRUE;
    }
  }

  function option_definition() {
    $options = parent::option_definition();

    $options['operator'] = array('default' => '=');
    $options['value'] = array('default' => '');
    $options['group'] = array('default' => '0');
    $options['exposed'] = array('default' => FALSE);
    $options['expose'] = array(
      'contains' => array(
        'operator' => array('default' => FALSE),
        'label' => array('default' => '', 'translatable' => TRUE),
      ),
    );

    return $options;
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
    $form['op_val_start'] = array('#value' => '<div class="clear-block">');
    $this->show_operator_form($form, $form_state);
    $this->show_value_form($form, $form_state);
    $form['op_val_end'] = array('#value' => '</div>');
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
    unset($form_state['values']['expose_button']); // don't store this.
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
    if (empty($this->no_operator)) {
      $form['value']['#prefix'] = '<div class="views-right-70">';
      $form['value']['#suffix'] = '</div>';
    }
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

    // When we click the expose button, we add new gadgets to the form but they
    // have no data in $_POST so their defaults get wiped out. This prevents
    // these defaults from getting wiped out. This setting will only be TRUE
    // during a 2nd pass rerender.
    if (!empty($form_state['force_expose_options'])) {
      foreach (element_children($form['expose']) as $id) {
        if (isset($form['expose'][$id]['#default_value']) && !isset($form['expose'][$id]['#value'])) {
          $form['expose'][$id]['#value'] = $form['expose'][$id]['#default_value'];
        }
      }
    }
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
    $form['expose']['start_left'] = array(
      '#value' => '<div class="views-left-50">',
    );

    $this->expose_form_left($form, $form_state);

    $form['expose']['end_left'] = array(
      '#value' => '</div>',
    );

    $form['expose']['start_checkboxes'] = array(
      '#value' => '<div class="form-checkboxes views-left-40 clear-block">',
    );

    $this->expose_form_right($form, $form_state);

    $form['expose']['end_checkboxes'] = array(
      '#value' => '</div>',
    );
  }

  /**
   * Handle the 'left' side fo the exposed options form.
   */
  function expose_form_left(&$form, &$form_state) {
    if (!empty($form['operator']['#type'])) {
      $form['expose']['use_operator'] = array(
        '#type' => 'checkbox',
        '#title' => t('Unlock operator'),
        '#description' => t('When checked, the operator will be exposed to the user'),
        '#default_value' => !empty($this->options['expose']['use_operator']),
      );
      $form['expose']['operator'] = array(
        '#type' => 'textfield',
        '#default_value' => $this->options['expose']['operator'],
        '#title' => t('Operator identifier'),
        '#size' => 40,
        '#description' => t('This will appear in the URL after the ? to identify this operator.'),
        '#process' => array('views_process_dependency'),
        '#dependency' => array(
          'edit-options-expose-use-operator' => array(1)
        ),
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
  }

  /**
   * Handle the 'right' side fo the exposed options form.
   */
  function expose_form_right(&$form, &$form_state) {
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

    if (!empty($form_state['values']['options']['expose']['identifier']) && $form_state['values']['options']['expose']['identifier'] == 'value') {
      form_error($form['expose']['identifier'], t('This identifier is not allowed.'));
    }
  }

  /**
   * Perform any necessary changes to the form exposes prior to storage.
   * There is no need for this function to actually store the data.
   */
  function expose_submit($form, &$form_state) { }

  /**
   * Provide default options for exposed filters.
   */
  function expose_options() {
    $this->options['expose'] = array(
      'use_operator' => FALSE,
      'operator' => $this->options['id'] . '_op',
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

    if (!empty($this->options['expose']['use_operator']) && !empty($this->options['expose']['operator'])) {
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

      if ($value != 'value') {
        unset($form['value']);
      }
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
    // Checkboxes don't work so well in exposed forms due to GET conversions.
    if ($form['#type'] == 'checkboxes') {
      if (empty($form['#no_convert']) || !empty($this->options['expose']['single'])) {
        $form['#type'] = 'select';
      }
      if (empty($this->options['expose']['single'])) {
        $form['#multiple'] = TRUE;
      }
    }
    if (!empty($this->options['expose']['single']) && isset($form['#multiple'])) {
      unset($form['#multiple']);
      $form['#size'] = NULL;
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
   * the behavior of this filter.
   */
  function accept_exposed_input($input) {
    if (empty($this->options['exposed'])) {
      return TRUE;
    }


    if (!empty($this->options['expose']['use_operator']) && !empty($this->options['expose']['operator']) && isset($input[$this->options['expose']['operator']])) {
      $this->operator = $input[$this->options['expose']['operator']];
    }

    if (!empty($this->options['expose']['identifier'])) {
      $value = $input[$this->options['expose']['identifier']];

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

  function store_exposed_input($input, $status) {
    if (empty($this->options['exposed']) || empty($this->options['expose']['identifier'])) {
      return TRUE;
    }

    if (empty($this->options['expose']['remember'])) {
      return;
    }

    // Figure out which display id is responsible for the filters, so we
    // know where to look for session stored values.
    $display_id = ($this->view->display_handler->is_defaulted('filters')) ? 'default' : $this->view->current_display;

    // shortcut test.
    $operator = !empty($this->options['expose']['use_operator']) && !empty($this->options['expose']['operator']);

    // false means that we got a setting that means to recuse ourselves,
    // so we should erase whatever happened to be there.
    if (!$status && isset($_SESSION['views'][$this->view->name][$display_id])) {
      $session = &$_SESSION['views'][$this->view->name][$display_id];
      if ($operator && isset($session[$this->options['expose']['operator']])) {
        unset($session[$this->options['expose']['operator']]);
      }

      if (isset($session[$this->options['expose']['identifier']])) {
        unset($session[$this->options['expose']['identifier']]);
      }
    }

    if ($status) {
      if (!isset($_SESSION['views'][$this->view->name][$display_id])) {
        $_SESSION['views'][$this->view->name][$display_id] = array();
      }

      $session = &$_SESSION['views'][$this->view->name][$display_id];

      if ($operator && isset($input[$this->options['expose']['operator']])) {
        $session[$this->options['expose']['operator']] = $input[$this->options['expose']['operator']];
      }

      $session[$this->options['expose']['identifier']] = $input[$this->options['expose']['identifier']];
    }
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
 * A special handler to take the place of missing or broken handlers.
 */
class views_handler_filter_broken extends views_handler_filter {
  function ui_name() {
    return t('Broken/missing handler');
  }

  function ensure_my_table() { /* No table to ensure! */ }
  function query() { /* No query to run */ }
  function options_form(&$form, &$form_state) {
    $form['markup'] = array(
      '#prefix' => '<div class="form-item description">',
      '#value' => t('The handler for this item is broken or missing and cannot be used. If a module provided the handler and was disabled, re-enabling the module may restore it. Otherwise, you should probably delete this item.'),
    );
  }

  /**
   * Determine if the handler is considered 'broken'
   */
  function broken() { return TRUE; }
}

/**
 * Simple filter to handle equal to / not equal to filters
 */
class views_handler_filter_equality extends views_handler_filter {
  // exposed filter options
  var $no_single = TRUE;

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

  function option_definition() {
    $options = parent::option_definition();

    $options['case'] = array('default' => TRUE);

    return $options;
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
      if (empty($this->options['expose']['use_operator']) || empty($this->options['expose']['operator'])) {
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
    $where = array();
    preg_match_all('/ (-?)("[^"]+"|[^" ]+)/i', ' ' . $this->value, $matches, PREG_SET_ORDER);
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

    if (!$where) {
      return;
    }

    if ($this->operator == 'word') {
      $where = '(' . implode(' OR ', $where) . ')';
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
 *
 * Definition items:
 * - label: (REQUIRED) The label for the checkbox.
 */
class views_handler_filter_boolean_operator extends views_handler_filter {
  // exposed filter options
  var $no_single = TRUE;
  // Don't display empty space where the operator would be.
  var $no_operator = TRUE;

  function construct() {
    $this->value_value = t('True');
    if (isset($this->definition['label'])) {
      $this->value_value = $this->definition['label'];
    }
    parent::construct();
  }

  function option_definition() {
    $options = parent::option_definition();

    $options['value']['default'] = FALSE;

    return $options;
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
    parent::expose_options();
    $this->options['expose']['operator'] = '';
    $this->options['expose']['label'] = $this->value_value;
    $this->options['expose']['optional'] = FALSE;
  }

  function query() {
    $this->ensure_my_table();
    $this->query->add_where($this->options['group'], "$this->table_alias.$this->real_field " . (empty($this->value) ? '=' : '<>') . " 0");
  }

}

/**
 * Simple filter to handle matching of multiple options selectable via checkboxes
 *
 * Definition items:
 * - numeric: If set to true, this item will use numeric operators instead of string.
 *
 */
class views_handler_filter_in_operator extends views_handler_filter {
  var $value_form_type = 'checkboxes';

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

  function expose_options() {
    parent::expose_options();
    $this->options['expose']['reduce'] = FALSE;
  }

  function expose_form_right(&$form, &$form_state) {
    parent::expose_form_right($form, $form_state);
    $form['expose']['reduce'] = array(
      '#type' => 'checkbox',
      '#title' => t('Limit list to selected items'),
      '#description' => t('If checked, the selected items presented to the user will be the only ones selected here.'),
      '#default_value' => !empty($this->options['expose']['reduce']), // safety
    );
  }

  function option_definition() {
    $options = parent::option_definition();

    $options['operator']['default'] = 'in';
    $options['value']['default'] = array();

    return $options;
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
    $options = $this->value_options;
    if (!empty($form_state['exposed']) && !empty($this->options['expose']['reduce'])) {
      $options = $this->reduce_value_options();
    }

    $form['value'] = array(
      '#type' => $this->value_form_type,
      '#title' => $this->value_title,
      '#options' => $options,
      '#default_value' => (array) $this->value,
      // These are only valid for 'select' type, but do no harm to checkboxes.
      '#multiple' => TRUE,
      '#size' => count($this->value_options) > 8 ? 8 : $this->value_options,
    );
  }

  /**
   * When using exposed filters, we may be required to reduce the set.
   */
  function reduce_value_options($input = NULL) {
    if (!isset($input)) {
      $input = $this->value_options;
    }

    // Because options may be an array of strings, or an array of mixed arrays
    // and strings (optgroups) or an array of objects, we have to
    // step through and handle each one individually.
    $options = array();
    foreach ($input as $id => $option) {
      if (is_array($option)) {
        $options[$id] = $this->reduce_value_options($option);
        continue;
      }
      else if (is_object($option)) {
        $keys = array_keys($option->choice);
        $key = array_shift($keys);
        if (isset($this->value[$key])) {
          $options[$id] = $option;
        }
      }
      else if (isset($this->value[$id])) {
        $options[$id] = $option;
      }
    }
    return $options;
  }

  function accept_exposed_input($input) {
    // A very special override because the All state for this type of
    // filter could have a default:
    if (empty($this->options['exposed'])) {
      return TRUE;
    }

    // If this is single and optional, this says that yes this filter will
    // participate, but using the default settings.
    if (!empty($this->options['expose']['single']) && !empty($this->options['expose']['optional'])) {
      $identifier = $this->options['expose']['identifier'];
      if ($input[$identifier] == 'All') {
        return TRUE;
      }
    }

    return parent::accept_exposed_input($input);
  }

  function value_submit($form, &$form_state) {
    $form_state['values']['options']['value'] = array_filter($form_state['values']['options']['value']);
  }

  function admin_summary() {
    if (!empty($this->options['exposed'])) {
      return t('exposed');
    }

    $this->get_value_options();

    if (!is_array($this->value)) {
      return;
    }

    if (count($this->value) == 1) {
      // If there is only one, show it as an =.
      $keys = $this->value;
      $key = array_shift($keys);

      if (isset($this->value_options[$key])) {
        $value = check_plain($this->value_options[$key]);
      }
      else {
        $value = t('Unknown');
      }

      return (substr($this->operator, 0, 3) != 'not' ? '=' : '<>') . ' ' . $value;
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
  function option_definition() {
    $options = parent::option_definition();

    $options['value'] = array(
      'contains' => array(
        'min' => array('default' => ''),
        'max' => array('default' => ''),
        'value' => array('default' => ''),
      ),
    );

    return $options;
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
      if (empty($this->options['expose']['use_operator']) || empty($this->options['expose']['operator'])) {
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
        '#title' => empty($form_state['exposed']) ? t('Value') : '',
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
        '#title' => empty($form_state['exposed']) ? t('Min') : '',
        '#size' => 30,
        '#default_value' => $this->value['min'],
      );
      $form['value']['max'] = array(
        '#type' => 'textfield',
        '#title' => empty($form_state['exposed']) ? t('And max') : t('And'),
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
  function option_definition() {
    $options = parent::option_definition();

    // value is already set up properly, we're just adding our new field to it.
    $options['value']['type']['default'] = 'date';

    return $options;
  }

  /**
   * Add a type selector to the value form
   */
  function value_form(&$form, &$form_state) {
    if (empty($form_state['exposed'])) {
      $form['value']['type'] = array(
        '#type' => 'radios',
        '#title' => t('Value type'),
        '#options' => array(
          'date' => t('A date in any machine readable format. CCYY-MM-DD HH:MM:SS is preferred.'),
          'offset' => t('An offset from the current time such as "+1 day" or "-2 hours and 30 minutes"'),
        ),
        '#default_value' => $this->value['type'],
      );
    }
    parent::value_form($form, $form_state);
  }

  function options_validate(&$form, &$form_state) {
    parent::options_validate($form, $form_state);

    if (!empty($form_state['values']['options']['expose']['optional'])) {
      // Who cares what the value is if it's exposed and optional.
      return;
    }

    $this->validate_valid_time($form['value'], $form_state['values']['options']['operator'], $form_state['values']['options']['value']);
  }

  function exposed_validate(&$form, &$form_state) {
    if (empty($this->options['exposed'])) {
      return;
    }

    if (!empty($this->options['expose']['optional'])) {
      // Who cares what the value is if it's exposed and optional.
      return;
    }

    $value = &$form_state['values'][$this->options['expose']['identifier']];
    if (!empty($this->options['expose']['use_operator']) && !empty($this->options['expose']['operator'])) {
      $operator = $form_state['values'][$this->options['expose']['operator']];
    }
    else {
      $operator = $this->operator;
    }

    $this->validate_valid_time($this->options['expose']['identifier'], $operator, $value);

  }

  /**
   * Validate that the time values convert to something usable.
   */
  function validate_valid_time(&$form, $operator, $value) {
    $operators = $this->operators();

    if ($operators[$operator]['values'] == 1) {
      $convert = strtotime($value['value']);
      if ($convert == -1 || $convert === FALSE) {
        form_error($form['value'], t('Invalid date format.'));
      }
    }
    else {
      $min = strtotime($value['min']);
      if ($min == -1 || $min === FALSE) {
        form_error($form['min'], t('Invalid date format.'));
      }
      $max = strtotime($value['max']);
      if ($max == -1 || $max === FALSE) {
        form_error($form['max'], t('Invalid date format.'));
      }
    }
  }

  function accept_exposed_input($input) {
    if (empty($this->options['exposed'])) {
      return TRUE;
    }

    if (!empty($this->options['expose']['identifier'])) {
      $value = &$input[$this->options['expose']['identifier']];
      $value['type'] = $this->value['type'];
    }

    // Don't filter if value(s) are empty.
    $operators = $this->operators();
    if (!empty($this->options['expose']['use_operator']) && !empty($this->options['expose']['operator'])) {
      $operator = $form_state['values'][$this->options['expose']['operator']];
    }
    else {
      $operator = $this->operator;
    }

    if ($operators[$operator]['values'] == 1) {
      if ($value['value'] == '') {
        return FALSE;
      }
    }
    else {
      if ($value['min'] == '' || $value['max'] == '') {
        return FALSE;
      }
    }

    return parent::accept_exposed_input($input);
  }

  function op_between($field) {
    if ($this->operator == 'between') {
      $a = intval(strtotime($this->value['min'], 0));
      $b = intval(strtotime($this->value['max'], 0));
    }
    else {
      $a = intval(strtotime($this->value['max'], 0));
      $b = intval(strtotime($this->value['min'], 0));
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
    $value = intval(strtotime($this->value['value'], 0));
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
class views_handler_filter_many_to_one extends views_handler_filter_in_operator {
  function init(&$view, &$options) {
    parent::init($view, $options);
    $this->helper = new views_many_to_one_helper($this);
  }

  function option_definition() {
    $options = parent::option_definition();

    $options['operator']['default'] = 'or';
    $options['value']['default'] = array();

    return $options;
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

  var $value_form_type = 'select';
  function value_form(&$form, &$form_state) {
    parent::value_form($form, $form_state);

    if (empty($form_state['exposed'])) {
      $this->helper->options_form($form, $form_state);
    }
  }

  /**
   * Override ensure_my_table so we can control how this joins in.
   * The operator actually has influence over joining.
   */
  function ensure_my_table() {
    $this->helper->ensure_my_table();
  }

  function query() {
    if (empty($this->value)) {
      return;
    }
    $this->helper->add_filter();
  }
}

/**
 * @}
 */
