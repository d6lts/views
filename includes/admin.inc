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
 * Display the edit view form.
 *
 * Since this form is more of a summary than a form, there's a lot more work
 * here than you might find in most forms.
 */
function theme_views_ui_edit_view($view) {
  $output = '';
  $save_button = drupal_get_form('views_ui_edit_view_form', $view);

  if (!empty($view->changed)) {
    if (!is_numeric($view->vid)) {
      $warning = t('This view has not yet been created and will not be entered into the database until you click <b>Save</b>, below.');
    }
    else {
      $warning = t('This view has been modified and changes will not saved to the database until you click <b>Save</b>, below.');
    }
    $output .= '<div class="warning">' . $warning . "</div>\n";
  }

  $table = views_fetch_data($view->base_table);
  $base_table = !empty($table['table']['base']['title']) ?
    $table['table']['base']['title'] : t('Unknown or missing table name');

  $output .= '<div class="views-basic-info">';
  $output .= t('View %name, displaying items of type <b>@base</b>.',
    array('%name' => $view->name, '@base' => $base_table));
  $output .= '</div>';
  views_include('tabs');
  $tabs = new views_tabset;

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
  $output .= '<div id="views-ajax-title">&nbsp;</div>';
  $output .= '<div id="views-ajax-pad">';
  $output .= $message;
  $output .= '</div>';

  // Render the actual form here
  $output .= $save_button;

  views_add_css('admin');
  views_add_js('ajax');
  drupal_add_js('misc/jquery.form.js');

  $settings = array('views' => array('ajax' => array(
    'id' => '#views-ajax-pad',
    'title' => '#views-ajax-title',
    'defaultForm' => $message,
  )));
  drupal_add_js($settings, 'setting');

  return $output;
}

function views_ui_display_tab($view, &$display) {
  $plugin = views_fetch_plugin_data('display', $display->display_plugin);
  if (empty($plugin)) {
    return array(t('Invalid'), t("Error: Display @display refers to a plugin named '@plugin', but that plugin doesn't exist!", array('@display' => $display->id, '@plugin' => $display->display_plugin)));
  }

  if (empty($display->handler)) {
    $display->handler = views_get_plugin('display', $display->display_plugin);
    if (empty($display->handler)) {
      return FALSE;
    }

    // init the new display handler with data.
    $display->handler->init($view, $display);
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
    $left .= '<dt>' . t('View tag: !tag', array('!tag' => l($tag, "admin/build/views/nojs/details/$view->name", array('attributes' => array('class' => 'views-ajax-link'))))) . '</dt>';
  }

  $left .= $display->handler->options_summary();

  $left .= '</dl>';
  $middle .= '<dl>';
  $middle .= views_ui_add_info('relationship', $view);
  $middle .= views_ui_add_info('argument', $view);
  $middle .= views_ui_add_info('filter', $view);
  $middle .= '</dl>';

  $right .= '<dl>';
  $right .= views_ui_add_info('field', $view);
  $right .= views_ui_add_info('sort', $view);
  $right .= '</dl>';


  $body = '<div class="top"><div class="inside">' . $top . '</div></div>';
  $body .= '<div class="left tab-section"><div class="inside">' . $left . '</div></div>';
  $body .= '<div class="middle tab-section"><div class="inside">' . $middle . '</div></div>';
  $body .= '<div class="right tab-section"><div class="inside">' . $right . '</div></div>';

  $body = '<div class="views-display views-display-' . $display->id . (empty($display->deleted) ? '' : ' views-display-deleted') . '">' . $body . '</div>';
  return array($display->display_title, $body);
}

function views_ui_regenerate_tabs($view, $display_id = NULL) {
  if (empty($display_id)) {
    $displays = array_keys($view->display);
  }
  elseif (!is_array($display_id)) {
    $displays = array($display_id);
  }
  else {
    $displays = $display_id;
  }

  $object = new stdClass;
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
function views_ui_standard_form_buttons(&$form, $form_id) {
  $form['buttons'] = array(
    '#prefix' => '<div class="form-buttons">',
    '#suffix' => '</div>',
  );
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Accept'),
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
 * AJAX callback to add a display.
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
 * Form to add a display to a view.
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
  $plugin = views_fetch_plugin_data('display', $form_state['view']->display[$id]->display_plugin);
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
  if (empty($form_state['ajax'])) {
    drupal_set_message(t('Your changes have been saved.'));
    $form_state['redirect'] = 'admin/build/views/edit/' . $form_state['view']->name;
  }
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
    return views_ui_regenerate_tabs($form_state['view'], $display_id);
  }
}

/**
 * Form constructor callback to edit display of a view
 */
function views_ui_edit_display_form(&$form_state, $view, $display_id, $section) {
  $display = &$view->display[$display_id];
  if (empty($display->handler)) {
    $display->handler = views_get_plugin('display', $display->display_plugin);
    if (empty($display->handler)) {
      return array();
    }

    // init the new display handler with data.
    $display->handler->init($view, $display);
  }

  $form_state['view'] = $view;
  $form_state['display_id'] = $display_id;
  $form_state['section'] = $section;
  $display->handler->options_form($form, $form_state);
  views_ui_standard_form_buttons($form, 'views_ui_edit_display_form');
  return $form;
}

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

  // Remove the handler so that it doesn't get saved into the cache.
  unset($display->handler);
  views_ui_cache_set($form_state['view']);
  if (empty($form_state['ajax'])) {
    drupal_set_message(t('Your changes have been saved.'));
    $form_state['redirect'] = 'admin/build/views/edit/' . $form_state['view']->name;
  }
}

/**
 * Page callback to rearrange a section.
 */
function views_ui_rearrange_section($js, $view, $section, $display_id) {
  if (!$js) {
    $output = drupal_get_form('views_ui_rearrange_form', $view, $section, $display_id);
    return $output;
  }
  else {
    views_include('ajax');
    $form_state = views_ajax_form('views_ui_rearrange_form', t('Rearrange @section', array('@section' => $section)), $_GET['q'], $view, $section, $display_id);
    // regenerate the tabset, set it to replace
    return views_ui_regenerate_tabs($form_state['view'], $display_id);
  }
}

/**
 * Form to rearrange items in the views UI.
 */
function views_ui_rearrange_form(&$form_state, $view, $section, $display_id) {
  $form['#title'] = t('Rearrange @section', array('@section' => $section));
  $form['markup'] = array('#value' => '<p>' . t('rearrange form here') . '</p>');

  $form_state['view'] = $view;
  views_ui_standard_form_buttons($form, 'views_ui_rearrange_form');
  return $form;
}

/**
 * Submit handler for rearranging form
 */
function views_ui_rearrange_form_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/build/views/edit/' . $form_state['view']->name;
  dsm('submitted');
}

function views_ui_add_info($type, $view) {
  $output = '';

  foreach ($view->$type as $field) {
    $handler = views_get_handler($field->tablename, $field->field, $type);

    if (empty($handler)) {
      $output .= t("Error: handler for @table > @field doesn't exist!", array('@table' => $field->tablename, '@field' => $field->field));
      continue;
    }
    $output .= t('@group: @title', array('@group' => $handler->definition['group'], '@title' => $handler->definition['title'])) . '<br />';
  }

  if (empty($output)) {
    $output = '<p>' . t('None defined.') . '</p>';
  }

  $links = "<div class=\"links\">" . l(t('Add'), '') . " · " . l(t('Rearrange'), "admin/build/views/nojs/rearrange/$view->name/$type/display_id", array('attributes' => array('class' => 'views-ajax-link'))) . '</div>';

  return $links . '<dt>' . ucwords($type) . '</dt><dd>' . $output . '</dd>';
}
