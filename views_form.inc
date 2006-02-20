<?php

// ---------------------------------------------------------------------------
// Select Box Definitions

// These should probably have string array keys that are easier to identify.

/**
 * Select box entries for argument defaults.
 */
function _views_get_arguments_default() {
  return array(
    1 => t('Return Page Not Found'),
    2 => t('Display All Values'),
    3 => t('Summary Of Choices')
   );
}

/**
 * Types of views supported.
 */
function _views_types() {
  return array(
    'list' => t('List View'),
    'table' => t('Table View'),
    'teaser' => t('Teaser List'),
    'node' => t('Full Nodes'),
    'random teaser' => t('Random Teaser'),
    'random node' => t('Random Node')
  );
}

/**
 * Select box entries for sort ordering.
 */
function _views_sortorders() {
  return array(
    'ASC' => t('Ascending'),
    'DESC' => t('Descending')
  );
}

/**
 * Swap two items in an array.
 */
function _views_swap(&$arr, $a, $b) {
  $temp = $arr[$a];
  $arr[$a] = $arr[$b];
  $arr[$b] = $temp;
}

/**
 * Move an item up in an array.
 */
function _views_move_up(&$arr, $i) {
  if ($i <= 0 || $i >= count($arr)) {
    return; // can't do it.
  }
  _views_swap($arr, $i - 1, $i);
}

/**
 * Move an item down in an array.
 */
function _views_move_down(&$arr, $i) {
  if ($i >= count($arr) - 1 || $i < 0) {
    return; // can't do it.
  }
  _views_swap($arr, $i + 1, $i);
}

/**
 * Move an item to the front of an array.
 */
function _views_move_top(&$arr, $i) {
  if ($i <= 0 || $i >= count($arr)) {
    return; // can't do it.
  }
  $temp = $arr[$i];
  for ($x = $i; $x > 0; $x--)
    $arr[$x] = $arr[$x - 1];
  $arr[0] = $temp;
}

/**
 * Move an item to the end of an array.
 */
function _views_move_bottom(&$arr, $i) {
  $end = count($arr) - 1;
  if ($i >= $end || $i < 0) {
    return; // can't do it.
  }
  $temp = $arr[$i];
  for ($x = $i; $x < $end; $x++)
    $arr[$x] = $arr[$x + 1];
  $arr[$end] = $temp;
}

/**
 * Figure out which of the many, many buttons on a form were clicked and
 * handle it.
 */
function _views_check_sub_ops(&$form, &$order, $i) {

  if ($form['delete']['#value']) {
    unset($form['delete']['#value']);
    unset($order[$i]);
    $order = array_values($order); // reindex
    $form['delete']['#printed'] = true;
    $form['up']['#printed'] = true;
    $form['down']['#printed'] = true;
    $form['top']['#printed'] = true;
    $form['bottom']['#printed'] = true;
    return 'delete';
  }
  else foreach (array('up', 'down', 'top', 'bottom') as $dir) {
    if ($form[$dir]['#value']) {
      unset ($form[$dir]['#value']);
      $func = "views_move_$dir";
      $func($order, $i);
      return true;
    }
  }
  return false;
}

/**
 * Figure out if one of the add buttons on a form were clicked, and handle it.
 */
function _views_check_ops(&$view, $op, $form) {
  if ($op == t('Add Filter')) {
    $view->newfilter['field'] = $form['filter']['add']['field']['#value'];
    return 'filter';
  }
  else if ($op == t('Add Criteria')) {
    $view->newsort['field'] = $form['sort']['add']['field']['#value'];
    return 'sort';
  }
  else if ($op == t('Add Argument')) {
    $view->newargument['type'] = $form['argument']['add']['type']['#value'];
    return 'argument';
  }
  else if ($op == t('Add Field')) {
    $fieldbits = explode('.', $form['field']['add']['field']['#value']);
    $view->newfield['fullname'] = $form['field']['add']['field']['#value'];
    $view->newfield['tablename'] = $fieldbits[0];
    $view->newfield['field'] = $fieldbits[1];
    $view->newfield['label'] = $fieldnames[$form['field']['add']['field']['#value']];
    $view->newfield['queryname'] = "$fieldbits[0]_$fieldbits[1]";
    return 'field';
  }
}

/**
 * Custom form element to do our nice images.
 */
function views_elements() {
  $type['imagebutton'] = array('#input' => TRUE, '#button_type' => 'submit',);
  return $type;
}

function theme_imagebutton($element) {
  return '<input type="image" class="form-'. $element['#button_type'] .'" name="'. $element['#name'] .'" value="'. check_plain($element['#default_value']) .'" '. drupal_attributes($element['#attributes']) . ' src="' . $element['#image'] . '" alt="' . $element['#title'] . '" title="' . $element['#title'] . "\" />\n";
}

function imagebutton_value() {
  // null function guarantees default_value doesn't get moved to #value.
}

/**
 * Display all the guts of a view in a form for editing.
 */
function _views_view_form($view, $op = '') {
  _views_check_arrays($view); // make sure arrays that might be empty get set

  // Put in all our add buttons, then process them to see if they've been hit.
  $form['field']['add'] = array('#tree' => true, '#parents' => array('field', 'add'));

  $form['field']['add']['field'] = array(
    '#type' => 'select',
    '#options' => _views_get_fields(true),
  );
  $form['field']['add']['button'] = array(
    '#type' => 'button',
    '#value' => t('Add Field'),
  );

  views_ui_build_form($form['field']['add']);

  $form['argument']['add'] = array('#tree' => true, '#parents' => array('argument', 'add'));
  $form['argument']['add']['type'] = array(
    '#type' => 'select',
    '#options' => _views_get_arguments(true),
  );
  $form['argument']['add']['button'] = array(
    '#type' => 'button',
    '#value' => t('Add Argument'),
  );

  views_ui_build_form($form['argument']['add']);

  $form['filter']['add'] = array('#tree' => true, '#parents' => array('filter', 'add'));
  $form['filter']['add']['field'] = array(
    '#type' => 'select',
    '#default_value' => NULL,
    '#options' => _views_get_filters(true),
  );
  $form['filter']['add']['button'] = array(
    '#type' => 'button',
    '#value' => t('Add Filter'),
  );

  views_ui_build_form($form['filter']['add']);

  $form['sort']['add'] = array('#tree' => true,  '#parents' => array('sort', 'add'));
  $form['sort']['add']['field'] = array(
    '#type' => 'select',
    '#default_value' => NULL,
    '#options' => _views_get_sorts(true),
  );
  $form['sort']['add']['button'] = array(
    '#type' => 'button',
    '#value' => t('Add Criteria'),
  );

  views_ui_build_form($form['sort']['add']);
  
  $allbut = _views_check_ops($view, $op, $form);
  if ($_POST['edit'] && $op != t('Save')) {
    drupal_set_message(t('You have modified this view; changes will not be recorded until you Save the form.'));
  }

  foreach (array('field', 'argument', 'filter', 'sort') as $section) {
    if (views_ui_add_section($form[$section], $view, $section)) {
      $allbut = $section;
    }
  }

  $form['vid'] = array(
    '#type' => 'value',
    '#value' => $view->vid,
  );
  $form['allbut'] = array(
    '#type' => 'value',
    '#value' => $allbut,
  );
  $form['changed'] = array(
    '#type' => 'hidden',
    '#value' => $view->changed,
  );

  $form['basic-info'] = array(
    '#type' => 'fieldset',
    '#collapsible' => true,
    '#collapsed' => ($allbut != NULL),
    '#title' => t('Basic Information'),
  );

  $form['basic-info']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => $view->name,
    '#size' => 20,
    '#maxlength' => 32,
    '#description' => t('The unique identifier of the view; it is only important for overridden views and views that modules or themes will need to use.'),
    '#required' => true,
  );

  $form['basic-info']['access'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Access'),
    '#default_value' => $view->access,
    '#options' => views_handler_filter_role(),
    '#description' => t('Only the checked roles will be able to see this view in any form; if no roles are checked, access will not be restricted.'),
  );

  $form['basic-info']['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description'),
    '#default_value' => $view->description,
    '#size' => 60,
    '#maxlength' => 255,
    '#description' => t('A description of the view for the admin list.'),
  );

  // page info

  $form['page-info'] = array(
    '#type' => 'fieldset',
    '#collapsible' => true,
    '#collapsed' => ($allbut != NULL || !$view->page),
    '#title' => t('Page'),
  );

  $form['page-info']['page'] = array(
    '#type' => 'checkbox',
    '#title' => t('Provide Page View'),
    '#return_value' => 1,
    '#default_value' => $view->page,
    '#description' => t('If checked this view will be provided as a page. If not checked, the fields in this group will be ignored.'),
  );

  $form['page-info']['url'] = array(
    '#type' => 'textfield',
    '#title' => t('URL'),
    '#default_value' => $view->url,
    '#size' => 60,
    '#maxlength' => 255,
    '#description' => t("Enter the URL to use for this view in the form of 'dir/dir'. Do not begin or end the URL with a /. Example: 'view/tracker'. This is required if providing a page view."),
  );

  $form['page-info']['page_type'] = array(
    '#type' => 'select',
    '#title' => t('View Type'),
    '#default_value' => $view->page_type,
    '#options' => _views_types(),
    '#description' => t('How the nodes should be displayed to the user.'),
  );

  $form['page-info']['page_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $view->page_title,
    '#size' => 60,
    '#maxlength' => 255,
    '#description' => t('The title that be shown at the top of the view. May be blank. If your View allows arguments, you may use %1 to substitute argument 1 text, %2 to argument #2, and so on.'),
  );

  $form['page-info']['use_pager'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use Pager'),
    '#return_value' => 1,
    '#default_value' => $view->use_pager,
    '#description' => t('If checked this query may be multiple pages. If not checked this query will be one page.'),
  );
  $form['page-info']['breadcrumb_no_home'] = array(
    '#type' => 'checkbox',
    '#title' => t('Breadcrumb trail should not include "Home"'),
    '#return_value' => 1,
    '#default_value' => $view->breadcrumb_no_home,
    '#description' => t('If checked the breadcrumb trail for this page will discard "Home". Usually you will not set this, but this is used for the Front Page View, where it IS Home and should not leave a trail to itself.'),
  );
  $form['page-info']['nodes_per_page'] = array(
    '#type' => 'textfield',
    '#title' => t('Nodes per Page'),
    '#default_value' => $view->nodes_per_page,
    '#size' => 2,
    '#maxlength' => 2,
    '#description' => t('The number of nodes to display per page. If 0 no nodes will be displayed. If not using a pager, this will be the maximum number of nodes in the list.'),
    '#attributes' => NULL,
    '#required' => true,
  );
  $form['page-info']['menu-info'] = array(
    '#type' => 'fieldset',
    '#collapsible' => false,
    '#title' => t('Menu'),
  );

  $form['page-info']['menu-info']['menu'] = array(
    '#type' => 'checkbox',
    '#title' => t('Provide Menu'),
    '#return_value' => 1,
    '#default_value' => $view->menu,
    '#description' => t('If checked this view be given a menu entry in the Drupal menu system. If not checked the data in this group will be ignored.'),
  );

  $form['page-info']['menu-info']['menu_tab'] = array(
    '#type' => 'checkbox',
    '#title' => t('Provide Menu as Tab'),
    '#return_value' => 1,
    '#default_value' => $view->menu_tab,
    '#description' => t("If checked this view's menu entry will be provided as a tab rather than in the main menu system."),
  );

  $form['page-info']['menu-info']['menu_tab_default'] = array(
    '#type' => 'checkbox',
    '#title' => t('Make Default Menu Tab'),
    '#return_value' => 1,
    '#default_value' => $view->menu_tab_default,
    '#description' => t("If checked this view's menu entry will be provided as a tab, and will be the default tab for that URL path. For example, if the URL is 'tracker/all' and it is set as the default menu tab, it will be put into the menu as 'tracker' and 'tracker/all' will be the default tab. For tabs to work properly, one tab in the group must be set as the default."),
  );

  $form['page-info']['menu-info']['menu_tab_weight'] = array(
    '#type' => 'textfield',
    '#title' => t('Tab Weight'),
    '#default_value' => $view->menu_tab_weight,
    '#width' => 10,
    '#description' => t('If this is a menu tab, select the weight; lower numbers will be further to the left.'),
  );

  $form['page-info']['menu-info']['menu_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Menu Title'),
    '#default_value' => $view->menu_title,
    '#size' => 60,
    '#maxlength' => 255,
    '#description' => t('Enter the title to use for the menu entry or tab. If blank, the page title will be used.'),
  );


  $form['page-info']['page_header'] = array(
    '#type' => 'textarea',
    '#title' => t('Header'),
    '#default_value' => $view->page_header,
    '#cols' => 60,
    '#rows' => 6,
    '#description' => t('Text to display at the top of the view. May contain an explanation or links or whatever you like. Optional.'),
  );

  $form['page-info']['page_header_format'] = filter_form($view->page_header_format, 1, array('page_header_format'));

  // block info

  $form['block-info'] = array(
    '#type' => 'fieldset',
    '#collapsible' => true,
    '#collapsed' => ($allbut != NULL || !$view->block),
    '#title' => t('Block'),
  );

  $form['block-info']['block'] = array(
    '#type' => 'checkbox',
    '#title' => t('Provide Block'),
    '#return_value' => 1,
    '#default_value' => $view->block,
    '#description' => t('If checked this view will be provided as a block. If checked title may not be blank.'),
  );

  $form['block-info']['block_type'] = array(
    '#type' => 'select',
    '#title' => t('View Type'),
    '#default_value' => $view->block_type,
    '#options' => _views_types(),
    '#description' => t('How the nodes should be displayed to the user.'),
  );

  $form['block-info']['block_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $view->block_title,
    '#size' => 60,
    '#maxlength' => 255,
    '#description' => t('The title that will be shown at the top of the block. May be blank.'),
  );

  $form['block-info']['nodes_per_block'] = array(
    '#type' => 'textfield',
    '#title' => t('Nodes per Block'),
    '#default_value' => $view->nodes_per_block,
    '#size' => 2,
    '#maxlength' => 2,
    '#description' => t('If using a block, the maximum number of items to display in the block. Pagers are not used in blocks.'),
    '#attributes' => NULL,
  );

  $form['block-info']['block_more'] = array(
    '#type' => 'checkbox',
    '#title' => t('[More] Link?'),
    '#return_value' => 1,
    '#default_value' => $view->block_more,
    '#description' => t('If using a view as both a page and a block, display a more link in the block that links to the view URL?'),
  );

  $form['block-info']['block_use_page_header'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use Page Header'),
    '#return_value' => 1,
    '#default_value' => $view->block_use_page_header,
    '#description' => t('If checked, use the Page Header for block view instead. If so, you should leave the Block Header blank.'),
  );

  $form['block-info']['block_header'] = array(
    '#type' => 'textarea',
    '#title' => t('Header'),
    '#default_value' => $view->block_header,
    '#cols' => 60,
    '#rows' => 6,
    '#description' => t('Text to display at the top of the view. May contain an explanation or links or whatever you like. Optional.'),
  );

  $form['block-info']['block_header_format'] = filter_form($view->block_header_format, 1, array( 'block_header_format'));

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  if ($view->vid) {
    $form['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
    );
  }
  $form['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
  );

  return drupal_get_form('views_edit_view', $form);
}

/**
 * separately build a piece of the form
 */
function views_ui_build_form(&$form) {
  $form = form_builder('views_edit_view', $form);
}

/**
 * Add one of the list sections to the form.
 */
function views_ui_add_section(&$form, &$view, $section) {
  // add fields to the form.

  $form += array('#tree' => true, '#parents' => array($section));
  $view_section = &$view->$section;

  $num_items = count($view_section);
  // Populate the form with fields we need to check to track
  // changes through the form's life.
  $form['count'] = array('#type' => 'hidden', '#default_value' => $num_items);

  $form['order'] = array(
    '#type' => 'hidden',
    '#default_value' => $num_items ? implode(',', range(0, $num_items - 1)) : '',
  );

  // Run the builder to get the value on the form.
  views_ui_build_form($form);
  $order = $form['order']['#value'] != ''
    ? explode(',', $form['order']['#value'])
    : array();

  // Shortcut because operator precedence gets sticky if using $foo->$bar[]
  for ($i = $num_items; $i < $form['count']['#value']; $i++) {
    $view_section[] = array();
  }

  $newsection = "new$section";
  // instantiate the new field if one was added.
  if ($view->$newsection) {
    $view_section[] = $view->$newsection;
    $order[] = $form['count']['#value'];
    $form['count']['#value']++;
  }

  $func = "views_ui_add_$section";
  if ($order) {
    foreach ($order as $key => $i) {
      if ($func($form[$i], $view_section[$i], $order, $key, $i)) {
        $allbut = $section;
      }
    }
    $form['order']['#value'] = implode(',', $order);
  }
  return $allbut;
}

/**
 * Add all the info for a single field into the form.
 */
function views_ui_add_field(&$form, $field, &$order, $key, $i) {
  $form = array('#tree' => true, '#parents' => array('field', $i));
  $form['fullname'] = array(
    '#type' => 'hidden',
    '#default_value' => $field['fullname'],
  );

  views_ui_add_buttons($form);
  views_ui_build_form($form);

  $retval = _views_check_sub_ops($form, $order, $key);
  if ($retval == 'delete') {
    return true;
  }

  $fields = _views_get_fields();
  $fieldname = $form['fullname']['#value']; // combined table & fieldname
  $fieldinfo = $fields[$fieldname]; // laziness + readability

  $form['name'] = array(
    '#type' => 'markup',
    '#value' => $fieldinfo['name'], // combined table & fieldname
  );
  $form['queryname'] = array(
    '#type' => 'hidden',
    '#default_value' => $field['queryname'],
  );
  $form['tablename'] = array(
    '#type' => 'hidden',
    '#default_value' => $field['tablename'],
  );
  $form['field'] = array(
    '#type' => 'hidden',
    '#default_value' => $field['field'],
  );

  $form['label'] = array(
    '#type' => 'textfield',
    '#default_value' => $field['label'],
    '#size' => 15,
    '#maxlength' => 25,
  );

  if (is_array($fieldinfo['handler'])) {
    $form['handler'] = array(
      '#type' => 'select',
      '#default_value' => $field['handler'],
      '#options' => $fieldinfo['handler'],
    );
  }

  if ($fieldinfo['sortable']) {
    $form['sortable'] = array(
      '#type' => 'select',
      '#default_value' => $field['sortable'],
      '#options' => array('0' => t('No'), '1' => t('Yes')),
    );
    $form['defaultsort'] = array(
      '#type' => 'select',
      '#default_value' => $field['defaultsort'],
      '#options' => array('0' => t('None'), 'ASC' => t('Ascending'), 'DESC' => t('Descending')),
    );
  }
  if (isset($fieldinfo['help'])) {
    $form['help'] = array(
      '#type' => 'markup',
      '#value' => '<div class="description">' . $fieldinfo['help'] . '</div>',
    );
  }

  return $retval;
}

/**
 * Add all the info for a single argument into the form.
 */
function views_ui_add_argument(&$form, $argument, &$order, $key, $i) {
  $form = array('#tree' => true, '#parents' => array('argument', $i));

  $form['type'] = array(
    '#type' => 'hidden',
    '#default_value' => $argument['type'],
  );

  views_ui_add_buttons($form);
  views_ui_build_form($form);

  $retval = _views_check_sub_ops($form, $order, $key);
  if ($retval == 'delete') {
    return true;
  }

  $arguments = _views_get_arguments();
  $arg_type = $form['type']['#value'];
  $arginfo = $arguments[$arg_type];

  $form['name'] = array(
    '#value' => $arginfo['name'],
  );

  $form['argdefault'] = array(
    '#type' => 'select',
    '#default_value' => max(1, intval($argument['argdefault'])),
    '#options' => _views_get_arguments_default(),
  );
  $form['title'] = array(
    '#type' => 'textfield',
    '#default_value' => $argument['title'],
    '#size' => 10,
    '#maxlength' => 255,
  );
  if (isset($arginfo['option'])) {
    $widget = $arginfo['option'];
    if (is_string($widget['#options']) && function_exists($widget['#options'])) {
      $widget['#options'] = $widget['#options']('option', $arginfo['option']);
    }
    $widget['#default_value'] = $argument['options'];
    $form['options'] = $widget;
    dprint_r($widget);
  }

  if (isset($arginfo['help'])) {
    $form['help'] = array(
      '#type' => 'markup',
      '#value' => $arginfo['help'],
    );
  }
}

/**
 * Add all the info for a single filter into the form.
 */
function views_ui_add_filter(&$form, $filter, &$order, $key, $i) {
  $form = array('#tree' => true, '#parents' => array('filter', $i));
  $form['field'] = array(
    '#type' => 'hidden',
    '#default_value' => $filter['field'],
  );

  views_ui_add_buttons($form);
  views_ui_build_form($form);

  $retval = _views_check_sub_ops($form, $order, $key);
  if ($retval == 'delete') {
    return true;
  }

  $filters = _views_get_filters();
  $filtername = $form['field']['#value']; // combined table & filtername
  $filterinfo = $filters[$filtername];

  $form['name'] = array(
    '#type' => 'markup',
    '#value' => $filterinfo['name'],
  );

  // 'operator' is either an array or a handler
  $operator = $filterinfo['operator'];
  if (!is_array($operator) && function_exists($filterinfo['operator']))  {
    $operator = $filterinfo['operator']('operator', $filterinfo);
  }

  $form['operator'] = array(
    '#type' => 'select',
    '#default_value' => $filter['operator'],
    '#options' => $operator,
  );
  if (isset($filterinfo['option'])) {
    $widget = $filterinfo['option'];
    if (is_string($widget['#options']) && function_exists($widget['#options'])) {
      $widget['#options'] = $widget['#options']('option', $filterinfo['option']);
    }
    $widget['#default_value'] = $filter['options'];
    $form['options'] = $widget;
  }

  $form['value'] = $filterinfo['value'];
  if (!is_array($form['value']['#options']) && function_exists($form['value']['#options'])) {
    $form['value']['#options'] = $form['value']['#options']('value', $filterinfo);
  }

  if (isset($filterinfo['help'])) {
    $form['help'] = array(
      '#type' => 'markup',
      '#value' => $filterinfo['help'],
    );
  }
}

/**
 * Add all the info for a single sort into the form.
 */
function views_ui_add_sort(&$form, $sort, &$order, $key, $i) {
  $form = array('#tree' => true, '#parents' => array('sort', $i));
  $form['field'] = array(
    '#type' => 'hidden',
    '#default_value' => $sort['field'],
  );

  views_ui_add_buttons($form);
  views_ui_build_form($form);

  $retval = _views_check_sub_ops($form, $order, $key);
  if ($retval == 'delete') {
    return true;
  }

  $sorts = _views_get_sorts();
  $sortname = $form['field']['#value']; // combined table & sortname
  $sortinfo = $sorts[$sortname];

  $form['name'] = array(
    '#value' => $sortinfo['name'],
  );

  $form['sortorder'] = array(
    '#type' => 'select',
    '#title' => NULL,
    '#default_value' => $sort['sortorder'],
    '#options' => _views_sortorders(),
  );
  // option can be either 'string', 'integer', an array or a handler
  // that must return an array.
  if (isset($sortinfo['option'])) {
    $widget = $sortinfo['option'];
    if (is_string($widget['#options']) && function_exists($widget['#options'])) {
      $widget['#options'] = $widget['#options']('option', $sortinfo['option']);
    }
    $widget['#default_value'] = $sort['options'];
    $form['options'] = $widget;
  }
  if (isset($sortinfo['help'])) {
    $form['help'] = array(
      '#type' => 'markup',
      '#value' => $sortinfo['help'],
    );
  }
}

/**
 * Add the up, down, top, bottom, delete buttons to a form.
 */
function views_ui_add_buttons(&$form_item) {
  $form_item['delete'] = views_ui_add_button('user-trash.png', t('Delete'), t('Delete this item.'));
  $form_item['up'] = views_ui_add_button('go-up.png', t('Up'), t('Move this item up.'));
  $form_item['down'] = views_ui_add_button('go-down.png', t('Down'), t('Move this item down.'));
  $form_item['top'] = views_ui_add_button('go-top.png', t('Top'), t('Move this item to the top.'));
  $form_item['bottom'] = views_ui_add_button('go-bottom.png', t('Bottom'), t('Move this item to the bottom.'));
}

/**
 * Add a single button to a form.
 */
function views_ui_add_button($image, $name, $text) {
  $module_path = base_path() . drupal_get_path('module', 'views');

  return array(
    '#type' => 'imagebutton',
    '#image' => $module_path . '/' . $image,
    '#title' => $text,
    '#default_value' => $name,
  );
}

/**
 * Display the form built by _views_view_form()
 */
function theme_views_edit_view($form) {

  $output .= form_render($form['basic-info']);
  $output .= form_render($form['page-info']);
  $output .= form_render($form['block-info']);

  $allbut = $form['allbut']['#value'];

  // form_render wants a reference so use full array notation.

  // fields
  $collapsed = $allbut && $allbut != 'field';
  $group = views_ui_render_section($form['field'], array('name', 'label', 'handler', 'sortable', 'defaultsort'), array(t('Name'), t('Label'), t('Handler'), t('Sortable'), t('Default Sort')), 'fields', $collapsed);
  $group .= views_ui_render_section_add($form['field']['add'], array('field', 'button'), 'Add Field');

  $group .= '<p>'. t('Fields are only meaningful with List view and Table View; they allow you to choose which fields are presented and in what order.') .'</p>';
  $output .= theme('fieldset', array('#title' => t('Fields'), '#children' => $group,
    '#collapsible' => true, '#collapsed' => $collapsed,));


  // arguments
  $collapsed = $allbut && $allbut != 'argument';
  $group = views_ui_render_section($form['argument'], array('name', 'argdefault', 'title', 'options'), array(t('Argument Type'), t('Default'), t('Title'), t('Option')), 'arguments', $collapsed);
  
  $group .= views_ui_render_section_add($form['argument']['add'], array('type', 'button'), 'Add Argument');
  
  $group .= '<p>'. t('Arguments are parsed directly from the URL. They are not necessary to any given view, but allow flexibility.') .'</p>';
  $output .= theme('fieldset', array('#title' => t('Arguments'), '#children' => $group,
    '#collapsible' => true, '#collapsed' => $collapsed,));

  $collapsed = $allbut && $allbut != 'filter';
  $group = views_ui_render_section($form['filter'], array('name', 'operator', 'value', 'options'), array(t('Field'), t('Operator'), t('Value'), t('Option')), 'filters', $collapsed);
  
  $group .= views_ui_render_section_add($form['filter']['add'], array('field', 'button'), 'Add Filter');

  $group .= '<p>'. t('Filters allow you to select a subset of all the nodes to display. All Filters are ANDed together.') .'</p>';
  $output .= theme('fieldset', array('#title' => t('Filters'), '#children' => $group,
    '#collapsible' => true, '#collapsed' => $collapsed,));
  $group = '';

  $collapsed = $allbut && $allbut != 'sort';
  $group = views_ui_render_section($form['sort'], array('name', 'sortorder', 'options'), array(t('Field'), t('Order'), t('Option')), 'sort criteria', $collapsed);
  
  $group .= views_ui_render_section_add($form['sort']['add'], array('field', 'button'), 'Add criteria');
  $output .= theme('fieldset', array('#title' => t('Sort Criteria'), '#children' => $group,
    '#collapsible' => true, '#collapsed' => $collapsed,));

  $output .= form_render($form);
  return $output;
}

/**
 * Render one of the dynamic sections on our form.
 */
function views_ui_render_section(&$form, $items, $header, $section, &$collapsed) {
  // always do the buttons
  $items = array_merge($items, array('delete', 'up', 'down', 'top', 'bottom'));
  $num_items = count($items);

  $order = ($form['order']['#value'] != ''
    ? explode(',', $form['order']['#value']) 
    : array());

  foreach ($order as $key => $i) {
    $row = array();
    foreach ($items as $item) {
      $row[] = form_render($form[$i][$item]);
    }
    $rows[] = $row;
    if (isset($form[$i]['help'])) {
      $rows[] = array(array('data' => form_render($form[$i]['help']), 'colspan' => $num_items));
    }
  }
  if (!$rows) {
    $rows[] = array(array('data' => t('This view currently has no $section defined.'), 'colspan' => $num_items));
    $collapsed = true;
  }

  $header[] = array('data' => t('Ops'), 'colspan' => 5);
  return theme('table', $header, $rows) . "<br />";
}

/**
 * Render the add buttons for one of the dynamic sections on our form.
 */
function views_ui_render_section_add(&$form, $items,  $label) {
  foreach ($items as $item) {
    $row[] = form_render($form[$item]);
  }
  $rows[] = $row;

  $header = array(array('data' => t($label), 'colspan' => count($items)));
  return theme('table', $header, $rows) . "<br />";
}

/**
 * Validate that a view sent via form is OK.
 */
function views_edit_view_validate($form_id, $view) {
//  print_r(var_export($view, true));
  $filters = _views_get_filters();
//  _views_check_arrays($view);

  $op = $_POST['op'];

  if ($op != t('Save')) {
    return; // only validate on saving!
  }

  if ($view['vid']) {
    $changed = db_result(db_query("SELECT changed FROM {view_view} WHERE vid = $view[vid]"));
    if ($changed && $view['changed'] != $changed) {
      form_set_error('', t('Another user has modified this view, unable to save. You can get this error by using the Back button to re-edit a view after saving one; if you do this, be sure to Reload before making any changes!'));
      return;
    }
  }

  if ($view['use_block'] && !$view['title']) {
    form_set_error('title', t('If being used as a block, title may not be blank.'));
  }

  if (!$view['name']) {
    form_set_error('name', t('View name is required.'));
  }

  // test uniqueness of name
  $vid = db_result(db_query("SELECT vid FROM {view_view} WHERE name='%s'", $view['name']));
  if ($vid && $vid != $view['vid']) {
    form_set_error('name', t('View name already in use.'));
  }


  if ($view['use_block'] && $view['nodes_per_block'] < 1) {
    form_set_error('nodes_per_block', t('If being used as a block, Nodes Per Block must be positive.'));
  }
/*
  if ($view['type'] == 2 || $view['type'] == 1 && !count($view['field'])) { // "Table"
    form_set_error('add][field', "You must select what fields to display when using type 'Table'!");
  }
*/
}

function views_ui_reorder(&$view) {
  // re-order things as they were ordered on the form.
  $order = $view['order'] != '' ? explode(',', $view['order']) : array();
  foreach ($order as $position => $key) {
    $placeholder[] = $view[$key];
  }
  $view = $placeholder;

}

/**
 * Handle submit buttons on a view form.
 */
function views_edit_view_submit($form_id, $form) {
  $view = (object) $form;

  // Transform any data that requires it here.
  $view->access = array_keys(array_filter($view->access));

  // re-order things as they were ordered on the form.
  foreach (array('field', 'argument', 'filter', 'sort') as $section) {
    views_ui_reorder($view->$section);
  }

  _views_save_view($view);
  menu_rebuild();
  if ($form['vid']) {
    drupal_set_message(t('View successfully saved.'));
  }
  else {
    drupal_set_message(t('View successfully added.'));
  }

  return drupal_goto('admin/views');
}
