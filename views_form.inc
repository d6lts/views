<?php

// ---------------------------------------------------------------------------
// Select Box Definitions

// These should probably have string array keys that are easier to identify.

/*
 * Select box entries for argument defaults.
 */
function _views_get_arguments_default() {
  return array(
    1 => t('Return Page Not Found'),
    2 => t('Display All Values'),
    3 => t('Summary Of Choices')
   );
}

/*
 * Types of views supported.
 */
function _views_types() {
  return array(
    'list' => t("List View"), 
    'table' => t("Table View"), 
    'teaser' => t("Teaser List"),
    'node' => t("Full Nodes"),
    'random teaser' => t("Random Teaser"),
    'random node' => t("Random Node")
  );
}

/*
 * Select box entries for sort ordering.
 */
function _views_sortorders() {
  return array(
    "ASC" => t("Ascending"),
    "DESC" => t("Descending")
  );
}

/*
 * Swap two items in an array.
 */
function _views_swap(&$arr, $a, $b) {
  $temp = $arr[$a];
  $arr[$a] = $arr[$b];
  $arr[$b] = $temp;
}

/*
 * Move an item up in an array.
 */
function _views_move_up(&$arr, $i) {
  if ($i <= 0 || $i >= count($arr)) {
    return; // can't do it.
  }
  _views_swap($arr, $i - 1, $i);
}

/*
 * Move an item down in an array.
 */
function _views_move_down(&$arr, $i) {
  if ($i >= count($arr) - 1 || $i < 0) {
    return; // can't do it.
  }
  _views_swap($arr, $i + 1, $i);
}

/*
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

/*
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

/*
 * Figure out which of the many, many buttons on a form were clicked and
 * handle it.
 */
function _views_check_sub_ops(&$view, $var) {
  foreach ($view->$var as $i => $obj) {
    if (!$obj['op']) {
      continue;
    }
    if ($obj['op'] == t('Delete')) {
      unset($view->{$var}[$i]);
      $view->$var = array_values($view->$var); // reindex
      return true;
    }
    else if ($obj['op'] == t('Up')) {
      _views_move_up($view->$var, $i);
      _views_move_up($_POST['edit'][$var], $i);
      return true;
    }
    else if ($obj['op'] == t('Down')) {
      _views_move_down($view->$var, $i);
      _views_move_down($_POST['edit'][$var], $i);
      return true;
    }
    else if ($obj['op'] == t('Top')) {
      _views_move_top($view->$var, $i);
      _views_move_top($_POST['edit'][$var], $i);
      return true;
    }
    else if ($obj['op'] == t('Bottom')) {
      _views_move_bottom($view->$var, $i);
      _views_move_bottom($_POST['edit'][$var], $i);
      return true;
    }
  }
  return false;
}

/*
 * Figure out if one of the many, many buttons on a form were clicked, and
 * handle it.
 */
function _views_check_ops(&$view, $op) {
  if (!$op) {
    $checks = array('filter', 'sort', 'argument', 'field');
    foreach ($checks as $check) {
      if (_views_check_sub_ops($view, $check)) {
        return $check;
      }
    }
  }
  else if ($op == t("Add Filter")) {
    $filter['field'] = $view->add['filter'];
    $view->filter[] = $filter;
    return 'filter';
  }
  else if ($op == t("Add Criteria")) {
    $view->sort[] = $view->add['sort'];
    return 'sort';
  }
  else if ($op == t("Add Argument")) {
    $view->argument[] = $view->add['argument'];
    return 'argument';
  }
  else if ($op == t("Add Field")) {
    $fieldbits = explode('.', $view->add['field']);

    $newfield['fullname'] = $view->add['field'];
    $newfield['tablename'] = $fieldbits[0];
    $newfield['field'] = $fieldbits[1];
    $newfield['label'] = $fieldnames[$view->add['field']];
    $newfield['queryname'] = "$fieldbits[0]_$fieldbits[1]";
    $view->field[] = $newfield;
    return 'field';
  }
}

/*
 * Custom form element to do our nice images.
 */
function views_elements() {
  $type['imagebutton'] = array('#input' => TRUE, '#name' => 'op', '#button_type' => 'submit', '#form_submitted' => TRUE);
  return $type;
}

function theme_imagebutton($element) {
  return '<input type="image" class="form-'. $element['#button_type'] .'" name="'. $element['#name'] .'" value="'. check_plain($element['#value']) .'" '. drupal_attributes($element['#attributes']) ." />\n";
}

/*
 * Display all the guts of a view in a form for editing.
 */
function _views_view_form($view, $orig, $op = '') {
  if ($view) {
    _views_check_arrays($view); // make sure arrays that might be empty get set
    $allbut = _views_check_ops($view, $op);
    if ($orig->vid && $op != t('Save')) {
      drupal_set_message("You have modified this view; changes will not be recorded until you Save the form.");
    }
  }
  else
    $view = $orig;

  // Load some things we're going to need.
  $fields = _views_get_fields();
  $fieldnames = _views_get_fields(true); // for selecting

  $sorts = _views_get_sorts(); // for selecting
  $sortnames = _views_get_sorts(true); // for selecting

  $filters = _views_get_filters();
  $filternames = _views_get_filters(true);

  $arguments = _views_get_arguments();
  $argnames = _views_get_arguments(true);

  $form['vid'] = array(
    '#type' => 'value',
    '#value' => $orig->vid,
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
    // This may have to be enabled if/when checkboxes change their #default_values behavior.
//  foreach($view->access as $role) {
//    $access[$role] = true;
//  }

  $form['basic-info']['access'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Access'),
    '#default_value' => $view->access,
//    '#default_value' => $access,
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
    '#description' => t('Enter the URL to use for this view in the form of "dir/dir". Do not begin or end the URL with a /. Example: "view/tracker". This is required if providing a page view.'),
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
    '#description' => t("If this is a menu tab, select the weight; lower numbers will be further to the left."),
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

  $form['add'] = array('#tree' => true);

  $form['field'] = array('#tree' => true);

  foreach ($view->field as $i => $field) {
    $fieldname = $field['fullname']; // combined table & fieldname
    $fieldinfo = $fields[$fieldname]; // laziness + readability

    $form['field'][$i] = array('#tree' => true);

    $form['field'][$i]['name'] = array(
      '#type' => 'markup',
      '#value' => $fieldinfo['name'], // combined table & fieldname
    );
    $form['field'][$i]['queryname'] = array(
      '#type' => 'hidden',
      '#value' => $field['queryname'],
    );
    $form['field'][$i]['tablename'] = array(
      '#type' => 'hidden',
      '#value' => $field['tablename'],
    );
    $form['field'][$i]['field'] = array(
      '#type' => 'hidden',
      '#value' => $field['field'],
    );
    $form['field'][$i]['fullname'] = array(
      '#type' => 'hidden',
      '#value' => $field['fullname'],
    );

    $form['field'][$i]['label'] = array(
      '#type' => 'textfield',
      '#default_value' => $field['label'],
      '#size' => 15,
      '#maxlength' => 25,
    );

    if (is_array($fieldinfo['handler'])) {
      $form['field'][$i]['handler'] = array(
        '#type' => 'select',
        '#default_value' => $field['handler'],
        '#options' => $fieldinfo['handler'],
      );
    }

    if ($fieldinfo['sortable']) {
      $form['field'][$i]['sortable'] = array(
        '#type' => 'select',
        '#default_value' => $field['sortable'],
        '#options' => array('0' => t("No"), "1" => t("Yes")),
      );
      $form['field'][$i]['defaultsort'] = array(
        '#type' => 'select',
        '#default_value' => $field['defaultsort'],
        '#options' => array('0' => t("None"), "ASC" => t("Ascending"), "DESC" => t("Descending")),
      );
    }
    if (isset($fieldinfo['help'])) {
      $form['field'][$i]['help'] = array(
        '#type' => 'markup',
        '#value' => "<div class='description'>" . $fieldinfo['help'] . "</div>",
      );
    }
    $form['field'][$i]['delete'] = array(
      '#type' => 'imagebutton',
      '#name' => "edit[field][$i][op]",
      '#attributes' => array('src' => drupal_get_path('module', 'views') .'/user-trash.png', 'alt' => t('Delete this item'), 'title' => t('Delete this item')),
      '#value' => t("Delete"),
    );
    $form['field'][$i]['up'] = array(
      '#type' => 'imagebutton',
      '#name' => "edit[field][$i][op]",
      '#attributes' => array('src' => drupal_get_path('module', 'views') .'/go-up.png', 'alt' => t('Move item up'), 'title' => t('Move item up')),
      '#value' => t("Up"),
    );
    $form['field'][$i]['down'] = array(
      '#type' => 'imagebutton',
      '#name' => "edit[field][$i][op]",
      '#attributes' => array('src' => drupal_get_path('module', 'views') .'/go-down.png', 'alt' => t('Move item down'), 'title' => t('Move item down')),
      '#value' => t("Down"),
    );
    $form['field'][$i]['top'] = array(
      '#type' => 'imagebutton',
      '#name' => "edit[field][$i][op]",
      '#attributes' => array('src' => drupal_get_path('module', 'views') .'/go-top.png', 'alt' => t('Move item to top'), 'title' => t('Move item to top')),
      '#value' => t("Top"),
    );
    $form['field'][$i]['bottom'] = array(
      '#type' => 'imagebutton',
      '#name' => "edit[field][$i][op]",
      '#attributes' => array('src' => drupal_get_path('module', 'views') .'/go-bottom.png', 'alt' => t('Move item to bottm'), 'title' => t('Move item to bottm')),
      '#value' => t("Bottom"),
    );
  }

  $form['add']['field'] = array(
    '#type' => 'select',
    '#default_value' => NULL,
    '#options' => $fieldnames,
  );
  $form['add']['fieldbutton'] = array(
    '#type' => 'button',
    '#value' => t("Add Field"),
  );

  $form['argument'] = array('#tree' => true);
  
  foreach ($view->argument as $i => $argument) {
    $arginfo = $arguments[$argument['type']];
    $form['argument'][$i] = array('#tree' => true);

    $form['argument'][$i]['type'] = array(
      '#type' => 'select',
      '#default_value' => $argument['type'],
      '#options' => $argnames,
    );    
    $form['argument'][$i]['argdefault'] = array(
      '#type' => 'select',
      '#default_value' => intval($argument['argdefault']),
      '#options' => _views_get_arguments_default(),
    );
    $form['argument'][$i]['title'] = array(
      '#type' => 'textfield',
      '#default_value' => $argument['title'],
      '#size' => 10,
      '#maxlength' => 255,
    );
    if (isset($arginfo['option'])) {
      if ($arginfo['option'] == 'string') {
        $form['argument'][$i]['options'] = array(
          '#type' => 'textfield',
          '#default_value' => $argument['options'],
          '#size' => 10,
          '#maxlength' => 255,
        );
      }
      else if ($arginfo['option'] == 'integer') {
        // same as above for now but should have validate routine
        $form['argument'][$i]['options'] = array(
          '#type' => 'textfield',
          '#default_value' => $argument['options'],
          '#size' => 10,
          '#maxlength' => 255,
        );
      }
      else {
        $list = $arginfo['option'];
        if (!is_array($list) && function_exists($arginfo['option'])) {
          $list = $arginfo['option']('option');
        }
        $form['argument'][$i]['options'] = array(
          '#type' => 'select',
          '#default_value' => $argument['options'],
          '#options' => $list,
        );
        
      }
    }
    if (isset($arginfo['help'])) {
      $form['argument'][$i]['help'] = array(
        '#type' => 'markup',
        '#value' => $arginfo['help'],
      );
    }
    $form['argument'][$i]['delete'] = array(
      '#type' => 'imagebutton',
      '#name' => "edit[argument][$i][op]",
      '#attributes' => array('src' => drupal_get_path('module', 'views') .'/user-trash.png', 'alt' => t('Delete this item'), 'title' => t('Delete this item')),
      '#value' => t("Delete"),
    );
    $form['argument'][$i]['up'] = array(
      '#type' => 'imagebutton',
      '#name' => "edit[argument][$i][op]",
      '#attributes' => array('src' => drupal_get_path('module', 'views') .'/go-up.png', 'alt' => t('Move item up'), 'title' => t('Move item up')),
      '#value' => t("Up"),
    );
    $form['argument'][$i]['down'] = array(
      '#type' => 'imagebutton',
      '#name' => "edit[argument][$i][op]",
      '#attributes' => array('src' => drupal_get_path('module', 'views') .'/go-down.png', 'alt' => t('Move item down'), 'title' => t('Move item down')),
      '#value' => t("Down"),
    );
    $form['argument'][$i]['top'] = array(
      '#type' => 'imagebutton',
      '#name' => "edit[argument][$i][op]",
      '#attributes' => array('src' => drupal_get_path('module', 'views') .'/go-top.png', 'alt' => t('Move item to top'), 'title' => t('Move item to top')),
      '#value' => t("Top"),
    );
    $form['argument'][$i]['bottom'] = array(
      '#type' => 'imagebutton',
      '#name' => "edit[argument][$i][op]",
      '#attributes' => array('src' => drupal_get_path('module', 'views') .'/go-bottom.png', 'alt' => t('Move item to bottm'), 'title' => t('Move item to bottm')),
      '#value' => t("Bottom"),
    );
  }

  $form['add']['argument'] = array('#tree' => true);

  $form['add']['argument']['type'] = array(
    '#type' => 'select',
    '#default_value' => NULL,
    '#options' => $argnames,
  );
  $form['add']['argument']['argdefault'] = array(
    '#type' => 'select',
    '#default_value' => NULL,
    '#options' => _views_get_arguments_default(),
  );
  $form['add']['argument']['title'] = array(
    '#type' => 'textfield',
    '#default_value' => $argument['title'],
    '#size' => 10,
    '#maxlength' => 255,
  );
  $form['add']['argbutton'] = array(
    '#type' => 'button',
    '#value' => t("Add Argument"),
  );


  $form['filter'] = array('#tree' => true);

  foreach ($view->filter as $i => $filter) {
    $filtername = $filter['field']; // combined table & filtername
    $filterinfo = $filters[$filtername];

    $form['filter'][$i] = array('#tree' => true);

    $form['filter'][$i]['name'] = array(
      '#type' => 'markup',
      '#value' => $filterinfo['name'],
    );

    $form['filter'][$i]['field'] = array(
      '#type' => 'hidden',
      '#value' => $filter['field'],
    );

    // 'operator' is either an array or a handler
    $operator = $filterinfo['operator'];
    if (!is_array($operator) && function_exists($filterinfo['operator']))  {
      $operator = $filterinfo['operator']('operator', $filterinfo);
    }

    $form['filter'][$i]['operator'] = array(
      '#type' => 'select',
      '#default_value' => $filter['operator'],
      '#options' => $operator,
    );

    if (isset($filterinfo['option'])) {
      if ($filterinfo['option'] == 'string') {
        $form['filter'][$i]['options'] = array(
          '#type' => 'textfield',
          '#default_value' => $filter['options'],
          '#size' => 10,
          '#maxlength' => 255,
        );
      }
      else if ($filterinfo['option'] == 'integer') {
        // same as above for now but should have validate routine
        $form['filter'][$i]['options'] = array(
          '#type' => 'textfield',
          '#default_value' => $filter['options'],
          '#size' => 10,
          '#maxlength' => 255,
        );
      }
      else {
        $list = $filterinfo['option'];
        if (!is_array($list) && function_exists($filterinfo['option'])) {
          $list = $filterinfo['option']('filteroption');
        }
        $form['filter'][$i]['options'] = array(
          '#type' => 'select',
          '#default_value' => $filter['options'],
          '#options' => $list,
        );
        
      }
    }

    if ($filterinfo['list']) {
      $list = $filterinfo['list'];

      if (!is_array($list) && function_exists($filterinfo['list']))  {
        $list = $filterinfo['list']('list', $filterinfo);
      }

      $multiple = false;
      switch($filterinfo['list-type']) {
        case 'list':
        default:
          $multiple = true;
          // no break, fall thru
        case 'select':
          $form['filter'][$i]['value'] = array(
            '#type' => 'select',
            '#default_value' => $filter['value'],
            '#options' => $list,
            '#multiple' => $multiple,
          );
          break;
      }
    }
    else {
      if ($filterinfo['transform'] && function_exists($filterinfo['transform'])) {
        $filter['value'] = $filterinfo['transform']('out', $filter['value'], $filterinfo);
      }
      $form['filter'][$i]['value'] = array(
        '#type' => 'textfield',
        '#default_value' => $filter['value'],
        '#size' => 20,
        '#maxlength' => 255,
      );
    }
    if (isset($filterinfo['help'])) {
      $form['filter'][$i]['help'] = array(
        '#type' => 'markup',
        '#value' => $filterinfo['help'],
      );
    }
    $form['filter'][$i]['delete'] = array(
      '#type' => 'imagebutton',
      '#name' => "edit[filter][$i][op]",
      '#attributes' => array('src' => drupal_get_path('module', 'views') .'/user-trash.png', 'alt' => t('Delete this item'), 'title' => t('Delete this item')),
      '#value' => t("Delete"),
    );
    $form['filter'][$i]['up'] = array(
      '#type' => 'imagebutton',
      '#name' => "edit[filter][$i][op]",
      '#attributes' => array('src' => drupal_get_path('module', 'views') .'/go-up.png', 'alt' => t('Move item up'), 'title' => t('Move item up')),
      '#value' => t("Up"),
    );
    $form['filter'][$i]['down'] = array(
      '#type' => 'imagebutton',
      '#name' => "edit[filter][$i][op]",
      '#attributes' => array('src' => drupal_get_path('module', 'views') .'/go-down.png', 'alt' => t('Move item down'), 'title' => t('Move item down')),
      '#value' => t("Down"),
    );
    $form['filter'][$i]['top'] = array(
      '#type' => 'imagebutton',
      '#name' => "edit[filter][$i][op]",
      '#attributes' => array('src' => drupal_get_path('module', 'views') .'/go-top.png', 'alt' => t('Move item to top'), 'title' => t('Move item to top')),
      '#value' => t("Top"),
    );
    $form['filter'][$i]['bottom'] = array(
      '#type' => 'imagebutton',
      '#name' => "edit[filter][$i][op]",
      '#attributes' => array('src' => drupal_get_path('module', 'views') .'/go-bottom.png', 'alt' => t('Move item to bottm'), 'title' => t('Move item to bottm')),
      '#value' => t("Bottom"),
    );
  }

  $form['add']['filter'] = array(
    '#type' => 'select',
    '#default_value' => NULL,
    '#options' => $filternames,
  );
  $form['add']['filterbutton'] = array(
    '#type' => 'button',
    '#value' => t("Add Filter"),
  );

  $form['sort'] = array('#tree' => true);
  foreach ($view->sort as $i => $sort) {
    $sortname = $sort['field']; // combined table & sortname
    $sortinfo = $sorts[$sortname];
    $form['sort'][$i] = array('#tree' => true);
    $form['sort'][$i]['field'] = array(
      '#type' => 'select',
      '#title' => NULL,
      '#default_value' => $sort['field'],
      '#options' => $sortnames,
    );
    $form['sort'][$i]['sortorder'] = array(
      '#type' => 'select',
      '#title' => NULL,
      '#default_value' => $sort['sortorder'],
      '#options' => _views_sortorders(),
    );
    // option can be either 'string', 'integer', an array or a handler
    // that must return an array.
    if (isset($sortinfo['option'])) {
      if ($sortinfo['option'] == 'string') {
        $form['sort'][$i]['options'] = array(
          '#type' => 'textfield',
          '#default_value' => $sort['options'],
          '#size' => 10,
          '#maxlength' => 255,
        );
      }
      else if ($sortinfo['option'] == 'integer') {
        // same as above for now but should have validate routine
        $form['sort'][$i]['options'] = array(
          '#type' => 'textfield',
          '#default_value' => $sort['options'],
          '#size' => 10,
          '#maxlength' => 255,
        );
      }
      else {
        $list = $sortinfo['option'];
        if (!is_array($list) && function_exists($sortinfo['option'])) {
          $list = $sortinfo['option']('sortoption');
        }
        $form['sort'][$i]['options'] = array(
          '#type' => 'select',
          '#default_value' => $sort['options'],
          '#options' => $list,
        );
        
      }
    }
    if (isset($sortinfo['help'])) {
      $form['sort'][$i]['help'] = array(
        '#type' => 'markup',
        '#value' => $sortinfo['help'],
      );
    }
    $form['sort'][$i]['delete'] = array(
      '#type' => 'imagebutton',
      '#name' => "edit[sort][$i][op]",
      '#attributes' => array('src' => drupal_get_path('module', 'views') .'/user-trash.png', 'alt' => t('Delete this item'), 'title' => t('Delete this item')),
      '#value' => t("Delete"),
    );
    $form['sort'][$i]['up'] = array(
      '#type' => 'imagebutton',
      '#name' => "edit[sort][$i][op]",
      '#attributes' => array('src' => drupal_get_path('module', 'views') .'/go-up.png', 'alt' => t('Move item up'), 'title' => t('Move item up')),
      '#value' => t("Up"),
    );
    $form['sort'][$i]['down'] = array(
      '#type' => 'imagebutton',
      '#name' => "edit[sort][$i][op]",
      '#attributes' => array('src' => drupal_get_path('module', 'views') .'/go-down.png', 'alt' => t('Move item down'), 'title' => t('Move item down')),
      '#value' => t("Down"),
    );
    $form['sort'][$i]['top'] = array(
      '#type' => 'imagebutton',
      '#name' => "edit[sort][$i][op]",
      '#attributes' => array('src' => drupal_get_path('module', 'views') .'/go-top.png', 'alt' => t('Move item to top'), 'title' => t('Move item to top')),
      '#value' => t("Top"),
    );
    $form['sort'][$i]['bottom'] = array(
      '#type' => 'imagebutton',
      '#name' => "edit[sort][$i][op]",
      '#attributes' => array('src' => drupal_get_path('module', 'views') .'/go-bottom.png', 'alt' => t('Move item to bottm'), 'title' => t('Move item to bottm')),
      '#value' => t("Bottom"),
    );
  }

  $form['add']['sort']['#tree'] = true;
  $form['add']['sort']['field'] = array(
    '#type' => 'select',
    '#default_value' => NULL,
    '#options' => $sortnames,
  );
  $form['add']['sort']['sortorder'] = array(
    '#type' => 'select',
    '#default_value' => NULL,
    '#options' => _views_sortorders(),
  );
  $form['add']['sortbutton'] = array(
    '#type' => 'button',
    '#value' => t("Add Criteria"),
  );

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t("Save"),
  );
  if ($orig->vid) {
    $form['delete'] = array(
      '#type' => 'submit',
      '#value' => t("Delete"),
    );
  }
  $form['cancel'] = array(
    '#type' => 'submit',
    '#value' => t("Cancel"),
  );
  
  return drupal_get_form('views_edit_view', $form);
}

/*
 * Display the form built by _views_view_form()
 */
function theme_views_edit_view($form) {
//  print "<pre>" . var_export($form, true) . "</pre>";

  $output .= form_render($form['basic-info']);
  $output .= form_render($form['page-info']);
  $output .= form_render($form['block-info']);

  // form_render wants a reference so use full array notation.

  $rows = array();
  $allbut = $form['allbut']['#value'];
  $collapsed = $allbut && $allbut != 'field';
  foreach ($form['field'] as $i => $field) {
    if (is_numeric($i)) {
      $rows[] = array(
        form_render($form['field'][$i]['name']),
        form_render($form['field'][$i]['label']),
        form_render($form['field'][$i]['handler']),
        form_render($form['field'][$i]['sortable']),
        form_render($form['field'][$i]['defaultsort']),
        form_render($form['field'][$i]['delete']),
        form_render($form['field'][$i]['up']),
        form_render($form['field'][$i]['down']),
        form_render($form['field'][$i]['top']),
        form_render($form['field'][$i]['bottom']),
      );
      if (isset($field['help'])) {
        $rows[] = array(array('data' => form_render($form['field'][$i]['help']), 'colspan' => '10'));
      }
    }
  }
  if (!$rows) {
    $rows[] = array(array('data' => "This view currently has no fields defined.", 'colspan' => 6));
    $collapsed = true;
  }
  
  $group .= theme('table', array('Name', 'Label', 'Handler', 'Sortable', 'Default Sort', array('data' =>'Ops', 'colspan' =>'6')), $rows);

  $rows = array(
    array(
      form_render($form['add']['field']),
      form_render($form['add']['fieldbutton']),
    )
  );
  $group .= "<br/>";
  $group .= theme('table', array(array('data' => t('Add Field'), 'colspan' => 3)), $rows);
  $group .= "<p>Fields are only meaningful with List view and Table View; they allow you to choose which fields are presented and in what order.</p>";
  $output .= theme('fieldset', array('#title' => t('Fields'), '#children' => $group, 
    '#collapsible' => true, '#collapsed' => $collapsed,));
  $group = "";

  $rows = array();
  $collapsed = $allbut && $allbut != 'argument';
  foreach ($form['argument'] as $i => $argument) {
    if (is_numeric($i)) {
      $rows[] = array(
        form_render($form['argument'][$i]['type']),
        form_render($form['argument'][$i]['argdefault']),
        form_render($form['argument'][$i]['title']),
        form_render($form['argument'][$i]['options']),
        form_render($form['argument'][$i]['delete']),
        form_render($form['argument'][$i]['up']),
        form_render($form['argument'][$i]['down']),
        form_render($form['argument'][$i]['top']),
        form_render($form['argument'][$i]['bottom']),
      );
      if (isset($argument['help'])) {
        $rows[] = array(array('data' => form_render($form['argument'][$i]['help']), 'colspan' => '10'));
      }
    }
  }
  if (!$rows) {
    $rows[] = array(array('data' => "This view currently has no arguments defined.", 'colspan' => 3));
    $collapsed = true;
  }

  $group .= theme('table', array(t('Argument Type'), t('Default'), t('Title'), t('Option'), array('data' =>'Ops', 'colspan' =>'6')), $rows);

  $rows = array(
    array(
      form_render($form['add']['argument']['type']),
      form_render($form['add']['argument']['argdefault']),
      form_render($form['add']['argument']['title']),
      form_render($form['add']['argbutton']),
    )
  );
  $group .= "<br/>";
  $group .= theme('table', array(t('Add Argument'), t('Default'), array('data' => t('Title'), 'colspan' => 3)), $rows);
  $group .= "<p>Arguments are parsed directly from the URL. They are not necessary to any given view, but allow flexibility.</p>";
  $output .= theme('fieldset', array('#title' => t('Arguments'), '#children' => $group,
    '#collapsible' => true, '#collapsed' => $collapsed,));
  $group = "";

  $rows = array();
  $collapsed = $allbut && $allbut != 'filter';
  foreach ($form['filter'] as $i => $filter) {
    if (is_numeric($i)) {
      $rows[] = array(
        form_render($form['filter'][$i]['name']),
        form_render($form['filter'][$i]['operator']),
        form_render($form['filter'][$i]['options']),
        form_render($form['filter'][$i]['value']),
        form_render($form['filter'][$i]['delete']),
        form_render($form['filter'][$i]['up']),
        form_render($form['filter'][$i]['down']),
        form_render($form['filter'][$i]['top']),
        form_render($form['filter'][$i]['bottom']),
      );
      if (isset($filter['help'])) {
        $rows[] = array(array('data' => form_render($form['filter'][$i]['help']), 'colspan' => '10'));
      }
    }
  }
  if (!$rows) {
    $rows[] = array(array('data' => "This view currently has no filters defined.", 'colspan' => 3));
    $collapsed = true;
  }

  $group .= theme('table', array('Field', 'Operator', 'Option', 'Value', array('data' =>'Ops', 'colspan' =>'6')), $rows);

  $rows = array(
    array(
      form_render($form['add']['filter']),
      form_render($form['add']['filterbutton']),
    )
  );
  $group .= "<br/>";
  $group .= theme('table', array(array('data' => t('Add Filter'), 'colspan' => 3)), $rows);
  $group .= "<p>Filters allow you to select a subset of all the nodes to display. All Filters are ANDed together.</p>";
  $output .= theme('fieldset', array('#title' => t('Filters'), '#children' => $group,
    '#collapsible' => true, '#collapsed' => $collapsed,));
  $group = "";

  $rows = array();
  $collapsed = $allbut && $allbut != 'sort';
  foreach ($form['sort'] as $i => $sort) {
    if (is_numeric($i)) {
      $rows[] = array(
        form_render($form['sort'][$i]['field']),
        form_render($form['sort'][$i]['sortorder']),
        form_render($form['sort'][$i]['options']),
        form_render($form['sort'][$i]['delete']),
        form_render($form['sort'][$i]['up']),
        form_render($form['sort'][$i]['down']),
        form_render($form['sort'][$i]['top']),
        form_render($form['sort'][$i]['bottom']),
      );
      if (isset($sort['help'])) {
        $rows[] = array(array('data' => form_render($form['sort'][$i]['help']), 'colspan' => '10'));
      }
    }
  }
  if (!$rows) {
    $rows[] = array(array('data' => "This view currently has no sort criteria defined.", 'colspan' => 3));
    $collapsed = true;
  }

  $group .= theme('table', array('Field', 'Order', 'Option', array('data' =>'Ops', 'colspan' =>'6')), $rows);

  $rows = array(
    array(
      form_render($form['add']['sort']['field']),
      form_render($form['add']['sort']['sortorder']),
      form_render($form['add']['sortbutton']),
    )
  );
  $group .= "<br/>";
  $group .= theme('table', array(array('data' => t('Add Sort Criteria'), 'colspan' => 3)), $rows);
  $output .= theme('fieldset', array('#title' => t('Sort Criteria'), '#children' => $group,
    '#collapsible' => true, '#collapsed' => $collapsed,));
  $group = "";

  $output .= form_render($form);
  return $output;
}

/*
 * Validate that a view sent via form is OK.
 */

function views_edit_view_validate($form_id, $view) {
//  dprint_r(var_export($view, true));
  $filters = _views_get_filters();
//  _views_check_arrays($view);
  
  $op = $_POST['op'];

  if ($op != t('Save')) {
    return; // only validate on saving!
  }

  if ($view['vid']) {
    $changed = db_result(db_query("SELECT changed FROM {view_view} WHERE vid = $view[vid]"));
    if ($changed && $view['changed'] != $changed) {
      form_set_error('', "Another user has modified this view, unable to save. You can get this error by using the Back button to re-edit a view after saving one; if you do this, be sure to Reload before making any changes!");
      return;
    }
  }

  if ($view['use_block'] && !$view['title']) {
    form_set_error('title', "If being used as a block, title may not be blank.");
  }

  if (!$view['name']) {
    form_set_error('name', "View name is required.");
  }

  // test uniqueness of name
  $vid = db_result(db_query("SELECT vid FROM {view_view} WHERE name='%s'", $view['name']));
  if ($vid && $vid != $view['vid']) {
    form_set_error('name', "View name already in use.");
  }


  if ($view['use_block'] && $view['nodes_per_block'] < 1) {
    form_set_error('nodes_per_block', "If being used as a block, Nodes Per Block must be positive.");
  }
/*
  if ($view['type'] == 2 || $view['type'] == 1 && !count($view['field'])) { // "Table" 
    form_set_error('add][field', "You must select what fields to display when using type 'Table'!");
  }
*/
}

/*
 * Handle submit buttons on a view form.
 */
function views_edit_view_submit($form_id, $form) {
//  dprint_r($form);
  $view = (object) $form;

  $view->access = array_keys(array_filter($view->access));

  _views_save_view($view);
  menu_rebuild();
  if ($form['vid']) {
    drupal_set_message("View successfully saved.");
  }
  else {
    drupal_set_message("View successfully added.");
  }

  return drupal_goto('admin/views');
}
