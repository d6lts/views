<?php
// $Id$

function _views_theme_functions($hook, $view, $display = NULL) {
  $themes = array();

  if ($display) {
    $themes[] = $hook . '__' . $display->id . '__' . $view->name;
    $themes[] = $hook . '__' . $display->id;
    if ($display->id != $display->display_plugin) {
      $themes[] = $hook . '__' . $display->display_plugin . '__' . $view->name;
      $themes[] = $hook . '__' . $display->display_plugin;
    }
  }
  $themes[] = $hook . '__' . $view->name;
  $themes[] = $hook;

  return $themes;
}

/**
 * Preprocess the primary theme implementation for a view.
 */
function template_preprocess_views_view(&$vars) {
  $view = $vars['view'];

  $vars['rows']       = $view->style_handler->render($view->result);
  $vars['css_name']   = views_css_safe($view->name);

  if (!$vars['rows']) {
    $vars['empty']    = $view->display_handler->render_empty();
    if (!$view->display_handler->get_option('header_empty')) {
      $vars['header'] = '';
    }
    if (!$view->display_handler->get_option('footer_empty')) {
      $vars['footer'] = '';
    }
  }
  else {
    $vars['empty']    = '';
    $header = TRUE;
  }

  $vars['exposed']    = !empty($view->exposed_widgets) ? $view->exposed_widgets : '';
  if (!isset($vars['header'])) {
    $vars['header']   = $view->display_handler->render_header();
  }
  if (!isset($vars['footer'])) {
    $vars['footer']   = $view->display_handler->render_footer();
  }
  $vars['more']       = $view->display_handler->render_more_link();
  $vars['feed_icon']  = $view->display_handler->render_feed_icon();

  $vars['pager']      = '';
  if (!empty($view->pager['use_pager'])) {
    $vars['pager']    = theme('pager', $view->exposed_input, $view->pager['items_per_page'], $view->pager['element']);
  }

}

/**
 * Preprocess theme function to print a single record from a row, with fields
 */
function template_preprocess_views_view_fields(&$vars) {
  $view = $vars['view'];

  // Loop through the fields for this view.
  foreach ($view->field as $field) {
    if (!empty($field['handler']) && is_object($field['handler'])) {
      $themes = array(
        'views_view_field__' . $view->name . '__' . $field['handler']->field_alias,
        'views_view_field__' . $view->name,
        'views_view_field__' . $field['handler']->field_alias,
        'views_view_field',
      );
      // Add the field into a variable named after the field. field_alias will be unique.
      $vars[$field['handler']->field_alias] = theme($themes, $view, $field, $vars['row']);
      // Create a second variable so we can easily find what fields we have and what the
      // CSS classes should be.
      $vars['fields'][$field['handler']->field_alias] = views_css_safe($field['handler']->field_alias);
    }
  }
}

/**
 * Display a single views field.
 *
 * Interesting bits of info:
 * $field->field_alias says what the raw value in $row will be. Reach it like
 * this: @code { $row->{$field->field_alias} @endcode
 */
function theme_views_view_field($view, $field, $row) {
  return $field['handler']->render($row);
}

/**
 * Preprocess theme function to print a single record from a row, with fields
 */
function template_preprocess_views_view_summary(&$vars) {
  $view     = $vars['view'];
  $argument = $view->argument[$view->build_info['summary_level']]['handler'];

  foreach ($vars['rows'] as $id => $row) {
    $vars['rows'][$id]->link = $argument->summary_name($row);
    $vars['rows'][$id]->url = $argument->summary_link($row, $view->get_url());
    $vars['rows'][$id]->count = intval($row->{$argument->count_alias});
  }
}

/**
 * Display a view as a table style.
 */
function template_preprocess_views_view_table(&$vars) {
  $view     = $vars['view'];
  $result   = $view->result;
  $options  = $view->style_handler->options;
  $handler  = $view->style_handler;

  $fields   = $view->field;
  $columns  = $handler->sanitize_columns($options['columns'], $fields);

  $active   = !empty($handler->active) ? $handler->active : '';
  $order    = !empty($handler->order) ? $handler->order : 'asc';

  $query    = tablesort_get_querystring();
  if ($query) {
    $query = '&' . $query;
  }

  foreach ($columns as $field => $column) {
    // render the header labels
    if ($field == $column) {
      if (empty($options['info'][$field]['sortable'])) {
        $vars['header'][$field] = check_plain($fields[$field]['handler']->label());
      }
      else {
        // @todo -- make this a setting
        $initial = 'asc';

        if ($active == $field && $order == 'asc') {
          $initial = 'desc';
        }

        $label = check_plain($fields[$field]['handler']->label());
        $image = theme('tablesort_indicator', $initial);
        $title = t('sort by @s', array('@s' => $label));
        $link_options = array(
          'html' => true,
          'attributes' => array('title' => $title),
          'query' => 'order=' . urlencode($field) . '&sort=' . $initial . $query,
        );
        $vars['header'][$field] = l($label . $image, $_GET['q'], $link_options);
      }
    }

    // Create a second variable so we can easily find what fields we have and what the
    // CSS classes should be.
    $vars['fields'][$field] = views_css_safe($field);
    if ($active == $field) {
      $vars['fields'][$field] .= ' active';
    }

    // Render each field into its appropriate column.
    foreach ($result as $num => $row) {
      if (!empty($fields[$field]['handler']) && is_object($fields[$field]['handler'])) {
        $handler = &$fields[$field]['handler'];
        $themes = array(
          'views_view_field__' . $view->name . '__' . $handler->field_alias,
          'views_view_field__' . $view->name,
          'views_view_field__' . $handler->field_alias,
          'views_view_field',
        );

        // Add the field into a variable named after the field. field_alias will be unique.
        if (isset($vars['rows'][$num][$column])) {
          if (!empty($options['info'][$column]['separator'])) {
            $vars['rows'][$num][$column] .= $options['info'][$column]['separator'];
          }
        }
        else {
          $vars['rows'][$num][$column] = '';
        }

        $vars['rows'][$num][$column] .= theme($themes, $view, $fields[$field], $row);
      }
    }
  }
}

/**
 * Default theme function for all filter forms.
 */
function template_preprocess_views_exposed_form(&$vars) {
  views_add_css('views');
  $form = &$vars['form'];

  // Put all single checkboxes together in the last spot.
  $checkboxes = '';

  $vars['widgets'] = array();
  foreach ($form['#info'] as $id => $info) {
    // Set aside checkboxes.
    if ($form[$info['value']]['#type'] == 'checkbox') {
      $checkboxes .= drupal_render($form[$info['value']]);
      continue;
    }
    $widget = new stdClass;
    // set up defaults so that there's always something there.
    $widget->label = $widget->operator = $widget->widget = NULL;

    if (!empty($info['label'])) {
      $widget->label = $info['label'];
    }
    if (!empty($info['operator'])) {
      $widget->operator = drupal_render($form[$info['operator']]);
    }
    $widget->widget = drupal_render($form[$info['value']]);
    $vars['widgets'][$id] = $widget;
  }

  // Wrap up all the checkboxes we set aside into a widget.
  if ($checkboxes) {
    $widget = new stdClass;
    // set up defaults so that there's always something there.
    $widget->label = $widget->operator = $widget->widget = NULL;
    $widget->widget = $checkboxes;
    $vars['widgets']['checkboxes'] = $widget;
  }

  // Don't render these:
  unset($form['form_id']);
  unset($form['form_build_id']);
  unset($form['form_token']);

  // This includes the submit button.
  $vars['button'] = drupal_render($form);
}

/**
 * @defgroup views_templates Views' template files
 */
