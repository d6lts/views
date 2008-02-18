<?php
// $Id$

/**
 * Preprocess the primary theme implementation for a view.
 */
function template_preprocess_views_view(&$vars) {
  $view = $vars['view'];

  $vars['rows']       = $view->style_handler->render($view->result);
  $vars['css_name']   = views_css_safe($view->name);

  if (!$vars['rows']) {
    $vars['empty']    = $view->display_handler->render_empty();
  }
  else {
    $vars['empty']    = '';
  }

  $vars['filters']    = $view->display_handler->render_filters();
  $vars['header']     = $view->display_handler->render_header();
  $vars['footer']     = $view->display_handler->render_footer();
  $vars['more']       = $view->display_handler->render_more_link();
  $vars['feed_icon']  = $view->display_handler->render_feed_icon();

  $vars['pager']      = '';
  if (!empty($view->pager['use_pager'])) {
    $vars['pager']    = theme('pager', array(), $view->pager['items_per_page'], $view->pager['element']);
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
 * @defgroup views_templates Views' template files
 */
