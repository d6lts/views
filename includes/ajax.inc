<?php
// $Id$

/**
 * @file ajax.inc
 *
 * Handles the server side AJAX interactions of Views.
 *
 * @defgroup ajax Views ajax library
 * @{
 */

/**
 * Simple render function to make sure output is what we want.
 */
function views_ajax_render($output = NULL, $title = NULL, $url = NULL, $js = NULL) {
  if (empty($output)) {
    $output->display = t('Server reports invalid input error.');
    $output->title = t('Error');
  }
  elseif (!is_object($output)) {
    $temp = new stdClass();
    $temp->display = $output;
    $temp->title = $title;
    $temp->url = $url;
    $output = $temp;
  }
  if (!empty($js)) {
    $output->js = $js;
  }

  drupal_set_header('Content-Type: text/javascript; charset=utf-8');
  print drupal_to_js($output);
  exit;
}


/**
 * Handle a form for AJAX in a manner that happens to be basically the
 * opposite of the normal flow; if the form hasn't been processed,
 * just render it and exit; if it has been submitted successfuly, however,
 * then we return whatever the submit function returned and do our
 * next step accordingly.
 *
 * Much of this code is from drupal_get_form and/or drupal_process_form.
 *
 * @param $form_id
 *   The id of the form
 * @param ...
 *   Any arguments that go to the form.
 */
function views_ajax_form($form_id) {
  $args = func_get_args();

  // Insert a placeholder for the form state so we can then assign a reference to it.
  array_splice($args, 1, 0, array(NULL));

  // Create the empty form state object.
  $form_state = array('storage' => NULL, 'submitted' => FALSE, 'post' => $_POST, 'ajax' => TRUE);

  // Replace the URL with the empty form state object as a reference:
  $args[1] = &$form_state;

  $form = call_user_func_array('drupal_retrieve_form', $args);

  // Prepare the form. My Drupal 5 code didn't do this, so I'm not sure
  // if it is necessary in D6 or not.
  $form_build_id = 'form-'. md5(mt_rand());
  $form['#build_id'] = $form_build_id;
  drupal_prepare_form($form_id, $form, $form_state);

  // Process the form our own selves so we don't get some extra batch handling
  // that we don't want.
  $form['#post'] = $_POST;
  $form_state['values'] = array();
  $form = form_builder($form_id, $form, $form_state);

  // from the POST data is set and matches the current form_id.
  if ((!empty($form['#post']) && (isset($form['#post']['form_id']) && ($form['#post']['form_id'] == $form_id)))) {
    drupal_validate_form($form_id, $form, $form_state);

    // In case of successful submit, execute the form handlers and return the
    // form state so the calling function can do what it needs to do next.
    if ((!empty($form_state['submitted'])) && !form_get_errors() && empty($form_state['rebuild'])) {
      $form_state['redirect'] = NULL;
      form_execute_handlers('submit', $form, $form_state);
      return $form_state;
    }
  }

  // If the form wasn't submitted successfully, render the form.
  $object = new stdClass();
  $object->display = '<div class="views-messages">' . theme('status_messages') . '</div>';
  $object->display .= drupal_render_form($form_id, $form);
  $object->title = empty($form['#title']) ? '' : $form['#title'];
  $object->url = empty($form['#url']) ? url($_GET['q'], array('absolute' => TRUE)) : $form['#url'];
  $object->js = empty($form['#js']) ? NULL : $form['#js'];
  if (!empty($form['#section'])) {
    $object->hilite = '.' . views_ui_item_css($form['#section']);
  }

  views_ajax_render($object);
}

/**
 * Sometimes we need to render a form but explicitly not process it.
 */
function views_render_ajax_form($form_id) {
  $args = func_get_args();

  // Insert a placeholder for the form state so we can then assign a reference to it.
  array_splice($args, 1, 0, array(NULL));

  // Create the empty form state object.
  $form_state = array('storage' => NULL, 'submitted' => FALSE, 'post' => $_POST, 'ajax' => TRUE);

  // Replace the URL with the empty form state object as a reference:
  $args[1] = &$form_state;

  $form = call_user_func_array('drupal_retrieve_form', $args);

  // Prepare the form. My Drupal 5 code didn't do this, so I'm not sure
  // if it is necessary in D6 or not.
  $form_build_id = 'form-'. md5(mt_rand());
  $form['#build_id'] = $form_build_id;
  drupal_prepare_form($form_id, $form, $form_state);

  // Process the form our own selves so we don't get some extra batch handling
  // that we don't want.
  $form['#post'] = array();
  $form_state['values'] = array();
  $form = form_builder($form_id, $form, $form_state);

  // Render the form and return the object data.
  $output = new stdClass();
  $object->display = '<div class="views-messages">' . theme('status_messages') . '</div>';
  $output->display .= drupal_render_form($form_id, $form);
  $output->title = empty($form['#title']) ? '' : $form['#title'];
  $output->url = empty($form['#url']) ? url($_GET['q'], array('absolute' => TRUE)) : $form['#url'];
  if ($form['#section']) {
    $output->hilite = '.' . views_ui_item_css($form['#section']);
  }

  return $output;
}

/**
 * @}
 */