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
  $view = views_new_view();
  return drupal_get_form('views_ui_add_form', $view);
}

/**
 * Form constructor callback to create the views Add Form, phase 1.
 */
function views_ui_add_form(&$form_state, $view) {
  $form = array();
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('View name'),
    '#description' => t('This is the unique name of the view. It must contain only alphanumeric characters and underscores; it is used to identify the view internally and to generate unique theming template names for this view. If overriding a module provided view, the name must not be changed or instead a new view will be created.'),
    '#default_value' => $view->name,
    '#required' => TRUE,
  );

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

  $form['base_table'] = array(
    '#type' => 'radios',
    '#title' => t('View type'),
    '#description' => t('The view type is the primary table for which information is being retrieved. The view type controls what arguments, fields, sort criteria and filters are available, so once this is set it <strong>cannot be changed</strong>.'),
    '#options' => views_fetch_base_table_names(),
    '#default_value' => $view->base_table,
    '#disabled' => !empty($view->vid) && $view->vid != 'new',
  );

  if (empty($view->vid) || $view->vid == 'new') {
    $form['displays'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Initial displays'),
      '#description' => t('Select which displays you would like to have automatically created with this view.'),
      '#options' => views_fetch_plugin_names('display'),
      '#required' => TRUE,
    );
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Next'),
    '#validate' => array('views_ui_add_form_validate'),
    '#submit' => array('views_ui_add_form_submit'),
  );

  $form_state['view'] = $view;

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
  $view = $form_state['view'];
  $view->name = $form_state['values']['name'];
  $view->description = $form_state['values']['description'];
  $view->tag = $form_state['values']['tag'];
  $view->base_table = $form_state['values']['base_table'];

  foreach (array_keys(array_filter($form_state['values']['displays'])) as $display_id) {
    // TODO: Should there be information in here to identify default style plugins
    // for displays?
    $view->add_display($display_id);
  }

  views_ui_cache_set($view);
  $form_state['redirect'] ='admin/build/views/edit/' . $view->name;
}

/**
 * The main view edit form
 */
function views_ui_edit_page($view) {
  drupal_set_title(t('Edit view "%view"', array('%view' => $view->name)));

  return drupal_get_form('views_ui_edit_view', $view);
}

function views_ui_edit_view(&$form_state, $view) {

/*
  $form['info'] = array(
    '#prefix' => '<div class="clear-block">',
    '#suffix' => '</div>',
  );
  $form['info']['base_table'] = array(
    '#type' => 'markup',
    '#value' => t('<b>View type:</b> @base', array('@base' => $base_table)),
    '#size' => 20,
  );

  if (!empty($table['table']['base']['help'])) {
    $form['info']['base_table_description'] = array(
      '#type' => 'markup',
      '#value' => '<div class="description">' . $table['table']['base']['help'] . '</div>',
      '#size' => 20,
    );
  }

  $form['info']['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description'),
    '#description' => t('This description will appear on the Views administrative UI to tell you what the view is about.'),
    '#default_value' => $view->description,
  );

  $form['info']['tag'] = array(
    '#type' => 'textfield',
    '#title' => t('View tag'),
    '#description' => t('Enter an optional tag for this view; it is used only to help sort views on the administrative page.'),
    '#default_value' => $view->tag,
    // TODO: This should be an autocomplete field.
  );
*/

  $form['display']['display'] = array(
    '#type' => 'select',
    '#options' => views_fetch_plugin_names('display'),
  );

  $form['display']['add_display'] = array(
    '#type' => 'submit',
    '#value' => t('Add display'),
  );

  $form['buttons']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#validate' => array('views_ui_edit_form_validate'),
    '#submit' => array('views_ui_edit_form_submit'),
  );

  $form['buttons']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#submit' => array('views_ui_edit_form_cancel'),
  );

  if (is_numeric($view->vid)) {
    $form['buttons']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#submit' => array('views_ui_edit_form_delete'),
    );
  }

  // The form_state isn't available
  $form['#view'] = $view;
  return $form;
}

/**
 * Display the edit view form.
 *
 * Since this form is more of a summary than a form, there's a lot more work
 * here than you might find in most forms.
 */
function theme_views_ui_edit_view($form) {
  $output = '';
  $view = $form['#view'];

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
  $output .= t('View <b>@name</b>, displaying items of type <b>@base</b>.',
    array('@name' => $view->name, '@base' => $base_table));
  $output .= '</div>';
  views_include('tabs');
  $tabs = new views_tabset;

  foreach ($view->display as $display) {
    $plugin = views_fetch_plugin_data('display', $display->display_plugin);
    if (empty($plugin)) {
      $output .= t("Error: Display @display refers to a plugin named '@plugin', but that plugin doesn't exist!", array('@display' => $display->id, '@plugin' => $display->display_plugin));
      continue;
    }
    // TODO: We probably need this settable on the display object itself.
    $title = $plugin['title'];

    // The first display is the default.
    if (empty($default_display)) {
      $title .= t(' (Default)');
      $default_display = TRUE;
    }

    $left = $middle = $right = '';

    $left = t('Basic display info goes here (TBD)');

    $middle = '<dl>';
    $middle .= views_ui_add_info('relationship', $view);
    $middle .= views_ui_add_info('argument', $view);
    $middle .= views_ui_add_info('filter', $view);
    $middle .= '</dl>';

    $right = '<dl>';
    $right .= views_ui_add_info('field', $view);
    $right .= views_ui_add_info('sort', $view);
    $right .= '</dl>';

    $body = '<div class="left tab-section"><div class="inside">' . $left . '</div></div>';
    $body .= '<div class="middle tab-section"><div class="inside">' . $middle . '</div></div>';
    $body .= '<div class="right tab-section"><div class="inside">' . $right . '</div></div>';
    $tabs->set($display->id, $title, $body);
  }

  // This is the area that will render beneath the links
  $tabs->add_extra(drupal_render($form['display']));
  $output .= $tabs->render();

  $output .= '<div class="views-ajax-pad">';
  $output .= '<div class="message">' . t("Click on an item to edit that item's details.") . '</div>';
  $output .= '</div>';

  // Render any of the form that is extra.
  $output .= drupal_render($form);

  views_add_css('admin');
  return $output;
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

  $links = "<div class=\"links\">" . l(t('Add'), '') . " · " . l(t('Rearrange'), '') . '</div>';

  return $links . '<dt>' . ucwords($type) . '</dt><dd>' . $output . '</dd>';
}