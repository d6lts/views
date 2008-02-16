<?php
//$Id$
/**
 * @file
 * Provide views data and handlers for user.module
 */

/**
 * @defgroup views_user_module user.module handlers
 *
 * @{
 */

/**
 * Implementation of hook_views_data()
 */
function user_views_data() {
  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.
  $data['users']['table']['group']  = t('User');

  $data['users']['table']['base'] = array(
    'field' => 'uid',
    'title' => t('User'),
    'help' => t('The node type allows views of your site\'s users.'),
  );

  $data['users']['table']['join']['node']['handler'] = 'views_join';
  $data['users']['table']['join']['node']['arguments'] = array('users', 'node', 'uid', 'uid');
  $data['users']['table']['group'] = t('User');

  // ----------------------------------------------------------------
  // Fields

  // name
  $data['users']['name'] = array(
    'title' => t('Name'), // The item it appears as on the UI,
    'help' => t('The user or author name.'), // The help that appears on the UI,
    'field' => array(
      'field' => 'name', // the real field
      'group' => t('User'), // The group it appears in on the UI,
      'handler' => 'views_handler_field_user_name',
      'click sortable' => TRUE,
    ),
  );
  // uid
  $data['users']['uid'] = array(
    'title' => t('Uid'),
    'help' => t('The user ID'), // The help that appears on the UI,
    // Information for displaying the uid
    'field' => array(
      'handler' => 'views_handler_field_user',
      'click sortable' => TRUE,
    ),
    // Information for accepting a uid as an argument
    'argument' => array(
      'handler' => 'views_handler_argument_uid',
    ), 
    // Information for accepting a uid as a filter
    'filter' => array(
      'handler' => 'views_handler_filter_equality',
    ),
    // Information for sorting on a uid.
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
  return $data;
}


/**
 * Field handler to provide simple renderer that allows linking to a user.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_user extends views_handler_field {
  /**
   * Override init function to provide generic option to link to user.
   */
  function init(&$view, &$data) {
    parent::init($view, $data);
    if (isset($data->options['link_to_user']) && $view->base_table != 'user') {
      $this->additional_fields[] = 'uid';
      $this->uid_field = 'users_uid';
    }
    else {
      $this->uid_field = 'uid';
    }
  }

  /**
   * Provide link to node option
   */
  function options_form(&$form) {
    $form['link_to_user'] = array(
      '#title' => t('Link this field to its user'),
      '#type' => 'checkbox',
      '#default_value' => $this->data->link_to_user,
    );
  }

  function render_link($data, $values) {
    if (!empty($this->data->link_to_user) && user_access('access user profiles')) {
      return l($data, "user/" . $values->{$this->uid_field}, array('html' => TRUE));
    }
    else {
      return $data;
    }
  }

  function render($values) {
    return $this->render_link(check_plain($values->{$this->field_alias}), $values);
  }
}

/**
 * Field handler to provide simple renderer that allows using a themed user link
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_user_name extends views_handler_field_user {

  function render_link($data, $values) {
    if (!empty($this->data->link_to_user)) {
      $account = (object)array('uid' => $values->{$this->uid_field}, 'name' => $values->{$this->field_alias});
      return theme('username', $account);
    }
    else {
      return $data;
    }
  }
}

/**
 * Argument handler to accept a user id.
 *
 * @ingroup views_argument_handlers
 */
class views_handler_argument_user_uid extends views_handler_argument {
  // No constructor is necessary.

  /**
   * Override the behavior of title(). Get the name of the user.
   */
  function title() {
    if (!$this->argument) {
      $title = variable_get('anonymous', t('Anonymous'));
    }
    else {
      $title = db_result(db_query("SELECT u.title FROM {user} u WHERE u.uid = %d"), $this->argument);
    }
    if (empty($title)) {
      return t('No title');
    }

    return check_plain($title);
  }
}


/**
 * @}
 */
