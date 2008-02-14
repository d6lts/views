<?php
// $Id$
/**
 * @file admin.inc
 * Provides the Views' administrative interface.
 */

/**
 * Page callback to list views in the system.
 */
function views_ui_list_views() {
  $items = array();
  $sorts = array();

  $header = array(
    array('data' => t('View name'), 'field' => 'name', 'sort' => 'asc'),
    array('data' => t('Description')),
    array('data' => t('Title'), 'field' => 'title'),
    array('data' => t('Type'), 'field' => 'type'),
    array('data' => t('URL'), 'field' => 'url'),
    array('data' => t('Operations')),
  );

  foreach (views_get_all_views() as $view) {
    $ops = array();
    if (empty($view->disabled)) {
      $ops[] = l(t('Edit'), "admin/build/views/edit/$view->name");
      $ops[] = l(t('Export'), "admin/build/views/$view->name/export");
    }
    if ($view->type != t('Default')) {
      $text = $view->type == t('Overridden') ? t('Revert') : t('Delete');
      $ops[] = l($text, "admin/build/views/$view->name/delete");
    }
    else {
      if (empty($view->disabled)) {
        $ops[] = l(t('Disable'), "admin/build/views/disable/$view->name", NULL, drupal_get_destination());
      }
      else {
        $ops[] = l(t('Enable'), "admin/build/views/enable/$view->name", NULL, drupal_get_destination());
      }
    }

  $path = 'Todo: path';
//    $path = empty($view->disabled) && strpos($view->path, '%') === FALSE ? l($view->path, $view->path) : check_plain($view->path);
    $item = array();
    $item[] = check_plain($view->name);
    $item[] = check_plain($view->description);
    $item[] = check_plain($view->get_title());
    $item[] = $view->type; // this is safe as it's always programmatic

    $item[] = $path;
    $item[] = implode(' | ', $ops);
    $items[] = $item;

    $ts = tablesort_init($header);
    switch ($ts['sql']) {
      case 'name':
      default:
        $sorts[] = $item[0];
        break;
      case 'title':
        $sorts[] = $item[1];
        break;
      case 'url':
        $sorts[] = 'todo: path'; // $path;
        break;
      case 'type':
        $sorts[] = $view->type . $item[0];
        break;
    }
  }

  if (strtolower($ts['sort']) == 'desc') {
    arsort($sorts);
  }
  else {
    asort($sorts);
  }

  $i = array();
  foreach ($sorts as $id => $title) {
    $i[] = $items[$id];
  }

  $output = theme('table', $header, $i);
  return $output;
}

/**
 * Page callback to add a new view.
 */
function views_ui_add_page() {
  return drupal_get_form('views_ui_add_form');
}

/**
 * Form constructor callback to create the views Add Form, phase 1.
 */
function views_ui_add_form(&$form_state) {
  $form = array();
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('View name'),
    '#description' => t('This is the unique name of the view. It must contain only alphanumeric characters and underscores; it is used to identify the view internally and to generate unique theming template names for this view. If overriding a module provided view, the name must not be changed or instead a new view will be created.'),
    '#required' => TRUE,
  );

  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => t('View description'),
    '#description' => t('This description will appear on the Views administrative UI to tell you what the view is about.'),
  );

  $form['tag'] = array(
    '#type' => 'textfield',
    '#title' => t('View tag'),
    '#description' => t('Enter an optional tag for this view; it is used only to help sort views on the administrative page.'),
    // TODO: This should be an autocomplete field.
  );

  $form['base_table'] = array(
    '#type' => 'radios',
    '#title' => t('View type'),
    '#description' => t('The view type is the primary table for which information is being retrieved. The view type controls what arguments, fields, sort criteria and filters are available, so once this is set it <strong>cannot be changed</strong>.'),
    '#default_value' => 'node', // default to node views.
    '#options' => views_fetch_base_table_names(),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Next'),
    '#validate' => array('views_ui_add_form_validate'),
    '#submit' => array('views_ui_add_form_submit'),
  );

  return $form;
}

/**
 * Validate the add view form.
 *
 * @todo Validate the name field.
 */
function views_ui_add_form_validate($form, &$form_state) {
}

/**
 * Process the add view form
 */
function views_ui_add_form_submit($form, &$form_state) {
  $view = views_new_view();
  $view->name = $form_state['values']['name'];
  $view->description = $form_state['values']['description'];
  $view->tag = $form_state['values']['tag'];
  $view->base_table = $form_state['values']['base_table'];

  views_ui_cache_set($view);
  $form_state['redirect'] ='admin/build/views/edit/' . $view->name;
}

/**
 * The main view edit page
 */
function views_ui_edit_page($view) {
  drupal_set_title(t('Edit view "%view"', array('%view' => $view->name)));
  return theme('views_ui_edit_view', $view);
}

/**
 * The main edit view form, which is really just a save/cancel/delete button.
 */
function views_ui_edit_view_form(&$form_state, $view) {
  $form['buttons']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#validate' => array('views_ui_edit_view_form_validate'),
    '#submit' => array('views_ui_edit_view_form_submit'),
  );

  $form['buttons']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#submit' => array('views_ui_edit_view_form_cancel'),
  );

  if (is_numeric($view->vid)) {
    $form['buttons']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#submit' => array('views_ui_edit_view_form_delete'),
    );
  }

  $form_state['view'] = $view;
  return $form;
}

/**
 * Submit handler for the edit view form.
 */
function views_ui_edit_view_form_submit($form, &$form_state) {
  $form_state['view']->save();
  drupal_set_message(t('The view has been saved.'));

  // Make sure menu items get rebuilt as neces
  menu_rebuild();

  // Clear the views cache.
  cache_clear_all('*', 'cache_views');

  // Clear the page cache.
  cache_clear_all();
}

/**
 * Display the edit view form.
 *
 * Since this form is more of a summary than a form, there's a lot more work
 * here than you might find in most forms.
 */
function theme_views_ui_edit_view($view) {
  $output = '';
  $save_button = drupal_get_form('views_ui_edit_view_form', $view);

  $table = views_fetch_data($view->base_table);
  $base_table = !empty($table['table']['base']['title']) ?
    $table['table']['base']['title'] : t('Unknown or missing table name');

  $output .= '<div class="views-basic-info">';
  if (!empty($view->changed)) {
    if (!is_numeric($view->vid)) {
      $output .= '<div class="view-changed view-new">' . t('New view') . '</div>';
    }
    else {
      $output .= '<div class="view-changed">' . t('Changed view') . '</div>';
    }
  }
  else {
    $output .= '<div class="view-changed view-hide">' . t('Changed view') . '</div>';
  }
  $output .= t('View %name, displaying items of type <b>@base</b>.',
    array('%name' => $view->name, '@base' => $base_table));
  $output .= '</div>';

  views_include('tabs');
  $tabs = new views_tabset;

  if (!$view->set_display('default')) {
    views_ajax_render(t('Invalid display id'));
  }

  foreach ($view->display as $display) {
    list($title, $body) = views_ui_display_tab($view, $display);
    // The first display is the default.
    $tabs->set($display->id, $title, $body);
  }

  // This is the area that will render beneath the links
  $display_button = drupal_get_form('views_ui_add_display_form', $view);
  $tabs->add_extra($display_button);

  $output .= $tabs->render();

  $message = '<div class="message">' . t("Click on an item to edit that item's details.") . '</div>';
  $output .= '<div id="views-ajax-form">';
  $output .= '<div id="views-ajax-title"></div>';
  $output .= '<div id="views-ajax-pad">';
  $output .= $message;
  $output .= '</div>';
  $output .= '</div>';

  // Render the actual form here
  $output .= $save_button;

  views_add_css('admin');
  views_add_js('ajax');
  drupal_add_js('misc/jquery.form.js');

  // Also add any js files required by plugins:
  $plugins = views_fetch_plugin_data();
  foreach ($plugins as $type => $type_plugins) {
    foreach ($type_plugins as $name => $plugin) {
      if (!empty($plugin['js'])) {
        foreach ($plugin['js'] as $file) {
          drupal_add_js($file);
        }
      }
    }
  }

  $settings = array('views' => array('ajax' => array(
    'id' => '#views-ajax-pad',
    'title' => '#views-ajax-title',
    'defaultForm' => $message,
  )));
  drupal_add_js($settings, 'setting');

  return $output;
}

/**
 * Generate the summary output for a single display to render in a tab.
 */
function views_ui_display_tab($view, &$display) {
  $plugin = views_fetch_plugin_data('display', $display->display_plugin);
  if (empty($plugin)) {
    return array(t('Invalid'), t("Error: Display @display refers to a plugin named '@plugin', but that plugin doesn't exist!", array('@display' => $display->id, '@plugin' => $display->display_plugin)));
  }

  // The display should always be initialized prior to this call.
  if (empty($display->handler)) {
    return FALSE;
  }

  $top = $left = $middle = $right = '';

  // If this form was submitted it was already handled, so force it not to
  // submit again.

  if (empty($plugin['no remove'])) {
    if (!empty($_POST['form_id']) && $_POST['form_id'] == 'views_ui_remove_display_form') {
      unset($_POST['form_id']);
    }
    $top .= drupal_get_form('views_ui_remove_display_form', $view, $display->id);
  }

  $top .= '<span class="display-title">' . check_plain($display->display_title) . '</span>';
  $top .= '<span class="display-description">' . check_plain($plugin['help']) . '</span>';

  $left .= '<dl>';
  // If this is the default display, add some basic stuff here.
  if ($display->id == 'default') {
    $tag = empty($view->tag) ? t('None') : $view->tag;
    $left .= '<dt>' . t('View settings') . '</dt>';
    $left .= '<dd class="views-item-details">' . t('Tag') . ': ' . l($tag, "admin/build/views/nojs/details/$view->name", array('attributes' => array('class' => 'views-ajax-link'))) . '</dd>';
  }

  $options = $categories = array();
  $display->handler->options_summary($categories, $options);

  // Build all of the options we were returned and put them into the
  // category data fields.
  foreach ($options as $id => $option) {
    if (empty($categories[$option['category']]['data'])) {
      $categories[$option['category']]['data'] = array();
    }
    $data = &$categories[$option['category']]['data'];
    $data[$id] = '';

    // If there are optional links, build them first so they float properly.
    if (!empty($option['links'])) {
      $links = array();
      foreach ($option['links'] as $link_id => $link_value) {
        $links[] = $display->handler->option_link($link_value, $link_id, 'views-button-configure');
      }
      $data[$id] .= implode(' ', $links);
    }
    if (!empty($option['title'])) {
      $data[$id] .= $option['title'] . ': ';
    }
    $data[$id] .= $display->handler->option_link($option['value'], $id);
  }

  // Once the data is built up, display the categories.
  foreach ($categories as $id => $category) {
    $left .= '<dt class="views-category-' . $id . '">' . $category['title'] . '</dt>';
    if (!empty($category['data'])) {
      foreach ($category['data'] as $cid => $cdata) {
        $left .= '<dd class="views-item-' . $cid . '">' . $cdata . '</dd>';
      }
    }
  }

  $left .= '</dl>';
  $middle .= '<dl>';
  $middle .= views_ui_add_info('relationship', $view, $display);
  $middle .= views_ui_add_info('argument', $view, $display);
  $middle .= views_ui_add_info('filter', $view, $display);
  $middle .= '</dl>';

  $right .= '<dl>';
  $right .= views_ui_add_info('field', $view, $display);
  $right .= views_ui_add_info('sort', $view, $display);
  $right .= '</dl>';


  $body = '<div class="top"><div class="inside">' . $top . '</div></div>';
  $body .= '<div class="left tab-section"><div class="inside">' . $left . '</div></div>';
  $body .= '<div class="middle tab-section"><div class="inside">' . $middle . '</div></div>';
  $body .= '<div class="right tab-section"><div class="inside">' . $right . '</div></div>';

  $body = '<div class="views-display views-display-' . $display->id . (empty($display->deleted) ? '' : ' views-display-deleted') . '">' . $body . '</div>';
  return array($display->display_title, $body);
}

function views_ui_regenerate_tabs($view, $display_id = NULL, $object = NULL) {
  if (empty($display_id)) {
    $displays = array_keys($view->display);
  }
  elseif (!is_array($display_id)) {
    $displays = array($display_id);
    if ($display_id != 'default') {
      $displays[] = 'default';
    }
  }
  else {
    $displays = $display_id;
  }

  if (!$view->set_display('default')) {
    views_ajax_render(t('Invalid display id'));
  }

  if (!is_object($object)) {
    $object = new stdClass();
  }

  $object->replace = array();
  foreach ($displays as $id) {
    list($title, $body) = views_ui_display_tab($view, $view->display[$id]);
    $object->replace['#views-tab-' . $id] = $body;
    $object->replace['#views-tab-title-' . $id] = $title;
  }
  views_ajax_render($object);
}

/**
 * Provide standard buttons for the forms to make it easy. Also provide
 * a hidden op operator because the forms plugin doesn't seem to properly
 * provide which button was clicked.
 */
function views_ui_standard_form_buttons(&$form, $form_id, $name = NULL) {
  $form['buttons'] = array(
    '#prefix' => '<div class="clear-block"><div class="form-buttons">',
    '#suffix' => '</div></div>',
  );

  if (empty($name)) {
    $name = t('Update');
  }
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => $name,
    '#submit' => array($form_id . '_submit'),
  );

  $cancel_submit = function_exists($form_id . '_cancel') ? $form_id . '_cancel' : 'views_ui_standard_cancel';
  $form['buttons']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#submit' => array($cancel_submit),
  );
}

function views_ui_standard_cancel($form, &$form_state) {
  $form_state['redirect'] = 'admin/build/views/edit/' . $form_state['view']->name;
}

// --------------------------------------------------------------------------
// Various subforms for editing the pieces of a view.

/**
 * AJAX callback to add a display.
 */
function views_ui_add_display($js, $view) {
  if (!$js) {
    return drupal_get_form('views_ui_add_display_form', $view);
  }

  views_include('ajax');
  $form_state = views_ajax_form('views_ui_add_display_form', $view);
  $view = $form_state['view'];
  $id = $form_state['id'];

  if (!$view->set_display('default')) {
    views_ajax_render(t('Invalid display id'));
  }

  list($title, $body) = views_ui_display_tab($view, $view->display[$id]);

  $output = new stdClass;
  $output->tab = array('#views-tab-' . $id => array('title' => $title, 'body' => $body));

  views_ajax_render($output);
}

/**
 * Form to add a display to a view.
 */
function views_ui_add_display_form(&$form_state, $view) {
  $form['display']['display'] = array(
    '#type' => 'select',
    '#options' => views_fetch_plugin_names('display'),
  );

  $form['display']['add_display'] = array(
    '#type' => 'submit',
    '#value' => t('Add display'),
    '#submit' => array('views_ui_add_display_form_submit'),
  );

  $form['#id'] = 'views-add-display-form';
  $form['#attributes'] = array('class' => 'views-ajax-form');
  $form['#action'] = url("admin/build/views/nojs/add-display/$view->name");

  $form_state['view'] = $view;
  return $form;
}

/**
 * Submit handler to add a display to a view.
 */
function views_ui_add_display_form_submit($form, &$form_state) {
  // Create the new display
  $plugin = $form_state['values']['display'];
  $form_state['id'] = $form_state['view']->add_display($plugin);

  // Store in cache
  views_ui_cache_set($form_state['view']);

  // Send it back
  $form_state['redirect'] = array('admin/build/views/edit/' . $form_state['view']->name, NULL, 'views-tab-' . $form_state['id']);
}

/**
 * AJAX callback to remove a display.
 */
function views_ui_remove_display($js, $view, $display_id) {
  if (!$js) {
    return drupal_get_form('views_ui_remove_display_form', $view, $display_id);
  }

  views_include('ajax');
  $form_state = views_ajax_form('views_ui_remove_display_form', $view, $display_id);
  // regenerate the tabset, set it to replace. Use $form_state['view'] here to get
  // the modifications.
  return views_ui_regenerate_tabs($form_state['view'], $display_id);
}

/**
 * Form to remove a display from a view.
 */
function views_ui_remove_display_form(&$form_state, $view, $display_id) {
  if (empty($view->display[$display_id]->deleted)) {
    $form['display'] = array(
      '#prefix' => '<div class="display-button remove-display">',
      '#suffix' => '</div>',
    );
    $form['remove_display'] = array(
      '#type' => 'submit',
      '#value' => t('Remove display'),
      '#submit' => array('views_ui_remove_display_form_submit'),
    );
  }
  else {
    $form['display'] = array(
      '#prefix' => '<div class="display-button restore-display">',
      '#suffix' => '</div>',
    );
    $form['restore_display'] = array(
      '#type' => 'submit',
      '#value' => t('Restore display'),
      '#submit' => array('views_ui_remove_display_form_restore'),
    );
  }
  $form['#action'] = url("admin/build/views/nojs/remove-display/$view->name/$display_id");
  $form['#id'] = 'views-add-display-form';
  $form['#attributes'] = array('class' => 'views-ajax-form');

  $form_state['view'] = $view;
  $form_state['display_id'] = $display_id;
  return $form;
}

/**
 * Submit handler to add a remove to a display from a view.
 */
function views_ui_remove_display_form_submit($form, &$form_state) {
  // Create the new display
  $plugin = views_fetch_plugin_data('display', $form_state['view']->display[$form_state['display_id']]->display_plugin);
  if (empty($plugin['no remove'])) {
    $id = $form_state['display_id'];
    $form_state['view']->display[$id]->deleted = TRUE;

    // Store in cache
    views_ui_cache_set($form_state['view']);
  }

  // Send it back
  $form_state['redirect'] = array('admin/build/views/edit/' . $form_state['view']->name, NULL, 'views-tab-' . $form_state['display_id']);
}

/**
 * Submit handler to add a restore a removed display to a view.
 */
function views_ui_remove_display_form_restore($form, &$form_state) {
  // Create the new display
  $id = $form_state['display_id'];
  $form_state['view']->display[$id]->deleted = FALSE;

  // Store in cache
  views_ui_cache_set($form_state['view']);

  // Send it back
  $form_state['redirect'] = array('admin/build/views/edit/' . $form_state['view']->name, NULL, 'views-tab-' . $form_state['display_id']);
}

/**
 * Page callback to edit details of a view.
 */
function views_ui_edit_details($js, $view) {
  if (!$js) {
    return drupal_get_form('views_ui_edit_details_form', $view);
  }
  else {
    views_include('ajax');
    $form_state = views_ajax_form('views_ui_edit_details_form', $view);
    return views_ui_regenerate_tabs($form_state['view']);
  }
}

/**
 * Form constructor callback to edit details of a view
 */
function views_ui_edit_details_form(&$form_state, $view) {
  $form['#title'] = t('View details');
  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => t('View description'),
    '#description' => t('This description will appear on the Views administrative UI to tell you what the view is about.'),
    '#default_value' => $view->description,
  );

  $form['tag'] = array(
    '#type' => 'textfield',
    '#title' => t('View tag'),
    '#description' => t('Enter an optional tag for this view; it is used only to help sort views on the administrative page.'),
    '#default_value' => $view->tag,
    // TODO: This should be an autocomplete field.
  );

  $form_state['view'] = $view;
  views_ui_standard_form_buttons($form, 'views_ui_edit_details_form');
  return $form;
}

/**
 * Submit handler for views_ui_edit_details_form
 */
function views_ui_edit_details_form_submit($form, &$form_state) {
  $form_state['view']->description = $form_state['values']['description'];
  $form_state['view']->tag = $form_state['values']['tag'];
  views_ui_cache_set($form_state['view']);
  $form_state['redirect'] = 'admin/build/views/edit/' . $form_state['view']->name;
}

/**
 * Page callback to edit options on a display of a view.
 */
function views_ui_edit_display($js, $view, $display_id, $section) {
  if (!$js) {
    return drupal_get_form('views_ui_edit_display_form', $view, $display_id, $section);
  }
  else {
    views_include('ajax');
    $form_state = views_ajax_form('views_ui_edit_display_form', $view, $display_id, $section);

    // Sometimes we need to re-generate the form for multi-step type operations.
    $object = NULL;
    if (!empty($form_state['regenerate form'])) {
      $object = views_render_ajax_form('views_ui_edit_display_form', $form_state['view'], $display_id, $section);
    }
    // regenerate all tabs because changes to the default tab could ripple.
    return views_ui_regenerate_tabs($form_state['view'], NULL, $object);
  }
}

/**
 * Form constructor callback to edit display of a view
 */
function views_ui_edit_display_form(&$form_state, $view, $display_id, $section) {
  if (!$view->set_display($display_id)) {
    views_ajax_render(t('Invalid display id'));
  }
  $display = &$view->display[$display_id];

  $form_state['view'] = $view;
  $form_state['display_id'] = $display_id;
  $form_state['section'] = $section;

  // Get form from the handler.
  $display->handler->options_form($form, $form_state);
  views_ui_standard_form_buttons($form, 'views_ui_edit_display_form');
  return $form;
}

/**
 * Validate handler for views_ui_edit_display_form
 */
function views_ui_edit_display_form_validate($form, &$form_state) {
  $display = &$form_state['view']->display[$form_state['display_id']];
  $display->handler->options_validate($form, $form_state);
}

/**
 * Submit handler for views_ui_edit_display_form
 */
function views_ui_edit_display_form_submit($form, &$form_state) {
  $display = &$form_state['view']->display[$form_state['display_id']];
  $display->handler->options_submit($form, $form_state);

  views_ui_cache_set($form_state['view']);
  // @todo: Need a safe function to use for drupal_set_message in an ajax environ.
  $form_state['redirect'] = 'admin/build/views/edit/' . $form_state['view']->name;
}

/**
 * Override handler for views_ui_edit_display_form
 */
function views_ui_edit_display_form_override($form, &$form_state) {
  $display = &$form_state['view']->display[$form_state['display_id']];
  $display->handler->options_override($form, $form_state);

  views_ui_cache_set($form_state['view']);
  $form_state['redirect'] = 'admin/build/views/edit/' . $form_state['view']->name;
  $form_state['regenerate form'] = TRUE;
}

/**
 * Page callback to rearrange a type.
 */
function views_ui_rearrange_type($js, $view, $display_id, $type) {
  if (!$js) {
    $output = drupal_get_form('views_ui_rearrange_form', $view, $type, $display_id);
    return $output;
  }
  else {
    views_include('ajax');
    $form_state = views_ajax_form('views_ui_rearrange_form', $view, $display_id, $type);
    // Sometimes we need to re-generate the form for multi-step type operations.
    $object = NULL;
    if (!empty($form_state['regenerate form'])) {
      $object = views_render_ajax_form('views_ui_rearrange_form', $form_state['view'], $display_id, $type);
    }

    // regenerate the tabset, set it to replace
    return views_ui_regenerate_tabs($form_state['view'], $display_id, $object);
  }
}

/**
 * Form to rearrange items in the views UI.
 */
function views_ui_rearrange_form(&$form_state, $view, $display_id, $type) {
  $types = views_object_types();
  $form['#title'] = t('Rearrange @type', array('@type' => $types[$type]['title']));
  $types = views_object_types();
  if (!$view->set_display($display_id)) {
    views_ajax_render(t('Invalid display id'));
  }
  $display = &$view->display[$display_id];

  if ($display->handler->defaultable_sections($types[$type]['plural'])) {
    $display->handler->add_override_button($form, $types[$type]['plural']);
    $form_state['section'] = $types[$type]['plural'];
  }

  $count = 0;

  foreach ($display->handler->get_option($types[$type]['plural']) as $id => $field) {
    $form[$id] = array('#tree' => TRUE);
    $form[$id]['weight'] = array(
      '#type' => 'weight',
      '#delta' => 10,
      '#default_value' => ++$count,
    );
    $handler = views_get_handler($field->table, $field->field, $type);
    if ($handler) {
      $handler->init($view, $field);
      $form[$id]['name'] = array(
        '#value' => t('@group: @title', array('@group' => $handler->definition['group'], '@title' => $handler->definition['title'])) . ' ' . $handler->admin_summary(),
      );
      $form[$id]['removed'] = array(
        '#type' => 'checkbox',
        '#id' => 'views-removed-' . $id,
        '#attributes' => array('class' => 'views-remove-checkbox'),
        '#default_value' => 0,
      );

    }
    else {
      $form[$id]['name'] = array('#value' => t('Broken field @id', $id));
    }
  }

  $form_state['view'] = $view;
  $form_state['display_id'] = $display_id;
  $form_state['type'] = $type;

  // Add javascript settings that will be added via $.extend for tabledragging
  $form['#js']['tableDrag']['arrange']['weight'][0] = array(
    'target' => 'weight',
    'source' => NULL,
    'relationship' => 'sibling',
    'action' => 'order',
    'hidden' => TRUE,
    'limit' => 0,
  );

  views_ui_standard_form_buttons($form, 'views_ui_rearrange_form');
  return $form;
}

/**
 * Turn the rearrange form into a proper table
 */
function theme_views_ui_rearrange_form($form) {
  $rows = array();
  foreach (element_children($form) as $id) {
    if (isset($form[$id]['name'])) {
      $row = array();
      $row[] = drupal_render($form[$id]['name']);
      $form[$id]['weight']['#attributes']['class'] = 'weight';
      $row[] = drupal_render($form[$id]['weight']);
      $row[] = drupal_render($form[$id]['removed']) . l('<span>' . t('Remove') . '</span>', 'javascript:void()', array('attributes' => array('id' => 'views-remove-link-' . $id, 'class' => 'views-button-remove views-remove-link', 'alt' => t('Remove this item'), 'title' => t('Remove this item')), 'html' => true));

      $rows[] = array('data' => $row, 'class' => 'draggable', 'id' => 'views-row-' . $id);
    }
  }
  if (empty($rows)) {
    $rows[] = array(array('data' => t('No fields available.'), 'colspan' => '2'));
  }

  $header = array('', t('Weight'), t('Remove'));
  drupal_add_tabledrag('arrange', 'order', 'sibling', 'weight');
  $output = drupal_render($form['override']);
  $output .= theme('table', $header, $rows, array('id' => 'arrange'));
  $output .= drupal_render($form);
  return $output;

}

/**
 * Submit handler for rearranging form
 */
function views_ui_rearrange_form_submit($form, &$form_state) {
  $types = views_object_types();
  $display = &$form_state['view']->display[$form_state['display_id']];

  $old_fields = $display->handler->get_option($types[$form_state['type']]['plural']);
  $new_fields = $order = array();

  // Make an array with the weights
  foreach ($form_state['values'] as $field => $info) {
    // add each value that is a field with a weight to our list, but only if
    // it has had its 'removed' checkbox checked.
    if (is_array($info) && isset($info['weight']) && empty($info['removed'])) {
      $order[$field] = $info['weight'];
    }
  }

  // Sort the array
  asort($order);

  // Create a new list of fields in the new order.
  foreach (array_keys($order) as $field) {
    $new_fields[$field] = $old_fields[$field];
  }
  $display->handler->set_option($types[$form_state['type']]['plural'], $new_fields);

  // Store in cache
  views_ui_cache_set($form_state['view']);

  $form_state['redirect'] = 'admin/build/views/edit/' . $form_state['view']->name;
}

/**
 * Page callback to add_item a type.
 */
function views_ui_add_item($js, $view, $display_id, $type) {
  if (!$js) {
    $output = drupal_get_form('views_ui_add_item_form', $view, $display_id, $type);
    return $output;
  }
  else {
    views_include('ajax');
    $form_state = views_ajax_form('views_ui_add_item_form', $view, $display_id, $type);
    // regenerate the tabset, set it to replace
    if (!empty($form_state['id'])) {
      $object = views_render_ajax_form('views_ui_config_item_form', $form_state['view'], $display_id, $type, $form_state['id']);
      // This form is at a different URL than $_GET['q'] so we have to pass via the object.
      $object->url = url("admin/build/views/ajax/config-item/$view->name/$display_id/$type/$form_state[id]");
    }
    return views_ui_regenerate_tabs($form_state['view'], $display_id, $object);
  }
}

/**
 * Form to add_item items in the views UI.
 */
function views_ui_add_item_form(&$form_state, $view, $display_id, $type) {
  if (!$view->set_display($display_id)) {
    views_ajax_render(t('Invalid display id'));
  }
  $display = &$view->display[$display_id];

  $types = views_object_types();
  $form['#title'] = t('Add @type', array('@type' => $types[$type]['title']));

  $form_state['view'] = &$view;
  $form_state['display_id'] = $display_id;
  $form_state['type'] = $type;

  $options = views_fetch_field_names($view->base_table, $type);

  if (!empty($options)) {
    $form['name'] = array(
      '#prefix' => '<div class="views-radio-box">',
      '#suffix' => '</div>',
      '#type' => 'checkboxes',
      '#options' => $options,
    );

    views_ui_standard_form_buttons($form, 'views_ui_add_item_form', t('Add'));
  }
  else {
    $form['markup'] = array(
      '#value' => t('There are no @types available to add.', array('@types' =>  strtolower($types[$type]['title']))),
    );
  }

  return $form;
}

/**
 * Submit handler for adding new item(s) to a view.
 */
function views_ui_add_item_form_submit($form, &$form_state) {
  $type = $form_state['type'];
  $types = views_object_types();

  // Loop through each of the items that were checked and add them to the view.
  foreach (array_keys(array_filter($form_state['values']['name'])) as $field) {
    list($table, $field) = explode('.', $field, 2);
    $id = $form_state['view']->add_item($form_state['display_id'], $type, $table, $field);
    if (empty($form_state['id'])) {
      $form_state['id'] = $id;
    }
  }

  // Store in cache
  views_ui_cache_set($form_state['view']);

  $form_state['redirect'] = 'admin/build/views/edit/' . $form_state['view']->name;
}

/**
 * Page callback to configure an item
 */
function views_ui_config_item($js, $view, $display_id, $type, $id) {
  if (!$js) {
    $output = drupal_get_form('views_ui_config_item_form', $view, $display_id, $type, $id);
    return $output;
  }
  else {
    views_include('ajax');
    $form_state = views_ajax_form('views_ui_config_item_form', $view, $display_id, $type, $id);
    // regenerate the tabset, set it to replace
    return views_ui_regenerate_tabs($form_state['view'], $display_id);
  }
}

/**
 * Form to config_item items in the views UI.
 */
function views_ui_config_item_form(&$form_state, $view, $display_id, $type, $id) {
  $form = array('options' => array('#tree' => TRUE));
  if (!$view->set_display($display_id)) {
    views_ajax_render(t('Invalid display id'));
  }
  $item = $view->get_item($display_id, $type, $id);

  if ($item) {
    $handler = views_get_handler($item->table, $item->field, $type);
    if (empty($handler)) {
      $form['markup'] = array('#value' => t("Error: handler for @table > @field doesn't exist!", array('@table' => $item->table, '@field' => $item->field)));
      break;
    }
    $handler->init($view, $item);
    $types = views_object_types();
    $form['#title'] = t('Configure @type "@group: @title"', array('@type' => strtolower($types[$type]['stitle']), '@group' => $handler->definition['group'], '@title' => $handler->definition['title']));

    // Get form from the handler.
    $handler->options_form($form['options'], $form_state);
    $form_state['view'] = &$view;
    $form_state['display_id'] = $display_id;
    $form_state['type'] = $type;
    $form_state['id'] = $id;
    $form_state['handler'] = &$handler;

    views_ui_standard_form_buttons($form, 'views_ui_config_item_form');
  }
  return $form;
}

/**
 * Submit handler for configing new item(s) to a view.
 */
function views_ui_config_item_form_validate($form, &$form_state) {
  $form_state['handler']->options_validate($form['options'], $form_state);
}

/**
 * Submit handler for configing new item(s) to a view.
 */
function views_ui_config_item_form_submit($form, &$form_state) {
  // Run it through the handler's submit function.
  $form_state['handler']->options_submit($form['options'], $form_state);
  $item = $form_state['handler']->data;

  // Store the data we're given.
  foreach ($form_state['values']['options'] as $key => $value) {
    $item->$key = $value;
  }

  // Store the item back on the view
  $form_state['view']->set_item($form_state['display_id'], $form_state['type'], $form_state['id'], $item);

  // Write to cache
  views_ui_cache_set($form_state['view']);
  $form_state['redirect'] = 'admin/build/views/edit/' . $form_state['view']->name;
}

/**
 * Page callback to change the summary style of an argument
 */
function views_ui_change_style($js, $view, $display_id, $type, $id) {
  if (!$js) {
    $output = drupal_get_form('views_ui_change_style_form', $view, $display_id, $type, $id);
    return $output;
  }
  else {
    views_include('ajax');
    $form_state = views_ajax_form('views_ui_change_style_form', $view, $display_id, $type, $id);
    // regenerate the tabset, set it to replace
    return views_ui_regenerate_tabs($form_state['view'], $display_id);
  }
}

/**
 * Form to change_style items in the views UI.
 */
function views_ui_change_style_form(&$form_state, $view, $display_id, $type, $id) {
  $form = array('options' => array('#tree' => TRUE));
  if (!$view->set_display($display_id)) {
    views_ajax_render(t('Invalid display id'));
  }
  $item = $view->get_item($display_id, $type, $id);

  if ($item) {
    $handler = views_get_handler($item->table, $item->field, $type);
    if (empty($handler)) {
      $form['markup'] = array('#value' => t("Error: handler for @table > @field doesn't exist!", array('@table' => $item->table, '@field' => $item->field)));
      break;
    }
    $handler->init($view, $item);
    $types = views_object_types();
    $form['#title'] = t('Change summary style for @type "@group: @title"', array('@type' => strtolower($types[$type]['stitle']), '@group' => $handler->definition['group'], '@title' => $handler->definition['title']));

    $form['style_plugin'] =  array(
      '#type' => 'radios',
      '#options' => views_fetch_plugin_names('style', 'summary', TRUE),
      '#default_value' => $item->style_plugin,
    );

    $form_state['view'] = &$view;
    $form_state['display_id'] = $display_id;
    $form_state['type'] = $type;
    $form_state['id'] = $id;
    $form_state['handler'] = &$handler;

    views_ui_standard_form_buttons($form, 'views_ui_change_style_form');
  }
  return $form;
}

/**
 * Submit handler for configing new item(s) to a view.
 */
function views_ui_change_style_form_submit($form, &$form_state) {
  // Run it through the handler's submit function.
  $form_state['handler']->options_submit($form['options'], $form_state);
  $item = $form_state['handler']->data;

  // Store the data we're given.
  $item->style_plugin = $form_state['values']['style_plugin'];

  // Store the item back on the view
  $form_state['view']->set_item($form_state['display_id'], $form_state['type'], $form_state['id'], $item);

  // Write to cache
  views_ui_cache_set($form_state['view']);
  $form_state['redirect'] = 'admin/build/views/edit/' . $form_state['view']->name;
}

/**
 * Page callback to config the summary style of an argument
 */
function views_ui_config_style($js, $view, $display_id, $type, $id) {
  if (!$js) {
    $output = drupal_get_form('views_ui_config_style_form', $view, $display_id, $type, $id);
    return $output;
  }
  else {
    views_include('ajax');
    $form_state = views_ajax_form('views_ui_config_style_form', $view, $display_id, $type, $id);
    // regenerate the tabset, set it to replace
    return views_ui_regenerate_tabs($form_state['view'], $display_id);
  }
}

/**
 * Form to config_style items in the views UI.
 */
function views_ui_config_style_form(&$form_state, $view, $display_id, $type, $id) {
  $form = array('options' => array('#tree' => TRUE));
  if (!$view->set_display($display_id)) {
    views_ajax_render(t('Invalid display id'));
  }
  $item = $view->get_item($display_id, $type, $id);

  if ($item) {
    $handler = views_get_handler($item->table, $item->field, $type);
    if (empty($handler)) {
      $form['markup'] = array('#value' => t("Error: handler for @table > @field doesn't exist!", array('@table' => $item->table, '@field' => $item->field)));
      break;
    }
    $handler->init($view, $item);
    $types = views_object_types();
    $form['#title'] = t('Configure summary style for @type "@group: @title"', array('@type' => strtolower($types[$type]['stitle']), '@group' => $handler->definition['group'], '@title' => $handler->definition['title']));

    $plugin = views_get_plugin('style', $item->style_plugin);
    if ($plugin) {
      $form['style_options'] = array(
        '#tree' => TRUE,
      );
      $plugin->init($view, $view->display[$display_id], $item->style_options);

      $plugin->options_form($form['style_options'], $form_state);
    }

    $form_state['view'] = &$view;
    $form_state['display_id'] = $display_id;
    $form_state['type'] = $type;
    $form_state['id'] = $id;
    $form_state['handler'] = &$handler;

    views_ui_standard_form_buttons($form, 'views_ui_config_style_form');
  }
  return $form;
}

/**
 * Submit handler for configing new item(s) to a view.
 */
function views_ui_config_style_form_submit($form, &$form_state) {
  // Run it through the handler's submit function.
  $form_state['handler']->options_submit($form['style_options'], $form_state);
  $item = $form_state['handler']->data;

  // Store the data we're given.
  $item->style_options = $form_state['values']['style_options'];

  // Store the item back on the view
  $form_state['view']->set_item($form_state['display_id'], $form_state['type'], $form_state['id'], $item);

  // Write to cache
  views_ui_cache_set($form_state['view']);
  $form_state['redirect'] = 'admin/build/views/edit/' . $form_state['view']->name;
}

/**
 * Add information about a section to a display.
 */
function views_ui_add_info($type, $view, $display) {
  $types = views_object_types();

  $output = '';
  $output .= l('<span>' . t('Rearrange') . '</span>', "admin/build/views/nojs/rearrange/$view->name/$display->id/$type", array('attributes' => array('class' => 'views-button-rearrange views-ajax-link'), 'html' => true));
  $output .= l('<span>' . t('Add') . '</span>', "admin/build/views/nojs/add-item/$view->name/$display->id/$type", array('attributes' => array('class' => 'views-button-add views-ajax-link'), 'html' => true));


  $output .= '<dt>' . $types[$type]['title'] . '</dt>';

  $fields = '';
  foreach ($display->handler->get_option($types[$type]['plural']) as $field) {
    $handler = views_get_handler($field->table, $field->field, $type);
    if (empty($handler)) {
      $output .= t("Error: handler for @table > @field doesn't exist!", array('@table' => $field->table, '@field' => $field->field)) . '<br/>';
      continue;
    }
    $handler->init($view, $field);

    $f = t('@group: @title', array('@group' => $handler->definition['group'], '@title' => $handler->definition['title']));
    $form = $form_state = array();
    $handler->options_form($form, $form_state);
    if (!empty($form)) {
      $f = l($f, "admin/build/views/nojs/config-item/$view->name/$display->id/$type/$field->id", array('attributes' => array('class' => 'views-ajax-link')));
    }
    $fields .=  '<dd class="views-item-' . $type . '-' . $field->id . '">' . $f . ' ' . $handler->admin_summary() . '</dt>';

    if ($handler->needs_style_plugin()) {
      $style_plugin = views_fetch_plugin_data('style', $handler->data->style_plugin);
      $style_title = empty($style_plugin['title']) ? t('Missing style plugin') : $style_plugin['title'];

      if (!empty($style_plugin['uses options'])) {
        $fields .= l('<span>' . t('Settings') . '</span>', "admin/build/views/nojs/config-style/$view->name/$display->id/$type/$field->id", array('attributes' => array('class' => 'views-button-configure views-ajax-link'), 'html' => true));
      }

      $fields .= '<dd class="views-item-style-plugin">' . t(' &nbsp; Style: !style', array('!style' => l($style_title, "admin/build/views/nojs/change-style/$view->name/$display->id/$type/$field->id", array('attributes' => array('class' => 'views-ajax-link'))))) . '</dd>';
    }
  }

  $output .= empty($fields) ? ('<dd>' . t('None defined.') . '</dd>') : $fields;
  return $output;
}

/**
 * Get a list of roles in the system.
 */
function views_ui_get_roles() {
  static $roles = NULL;
  if (!isset($roles)) {
    $roles = array();
    $result = db_query("SELECT r.rid, r.name FROM {role} r ORDER BY r.name");
    while ($obj = db_fetch_object($result)) {
      $roles[$obj->rid] = $obj->name;
    }
  }

  return $roles;
}