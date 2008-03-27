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
    'help' => t('Allows views of your site\'s users.'),
  );

  // @todo: restructure this back to normal.
  $data['users']['table']['join']['node']['handler'] = 'views_join';
  $data['users']['table']['join']['node']['arguments'] = array('users', 'node', 'uid', 'uid');

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
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    // @todo argument
  );

  // mail
  // Note that this field implements field level access control. Neato.
  $data['users']['mail'] = array(
    'title' => t('E-mail'), // The item it appears as on the UI,
    'help' => t('Email address for a given user. Only accessible to users with <em>administer users</em> permission'), // The help that appears on the UI,
    'field' => array(
      'field' => 'mail', // the real field
      'group' => t('User'), // The group it appears in on the UI,
      // TODO: mailto link?
      'handler' => 'views_handler_field_user_mail',
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
      'handler' => 'views_handler_argument_user_uid',
    ),
    // Information for accepting a uid as a filter
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    // Information for sorting on a uid.
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // @todo: users_roles table
  // @todo: filter on role
  // @todo roles field

  // @todo: tracker default view (in users.default_views.inc)
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
    if (!empty($this->options['link_to_user'])) {
      $this->additional_fields['uid'] = 'uid';
    }
  }

  function options(&$options) {
    parent::options($options);
    $options['link_to_user'] = TRUE;
  }

  /**
   * Provide link to node option
   */
  function options_form(&$form) {
    parent::options_form($form, $form_state);
    $form['link_to_user'] = array(
      '#title' => t('Link this field to its user'),
      '#type' => 'checkbox',
      '#default_value' => $this->options['link_to_user'],
    );
  }

  function render_link($data, $values) {
    if (!empty($this->options['link_to_user']) && user_access('access user profiles')) {
      return l($data, "user/" . $values->{$this->aliases['uid']}, array('html' => TRUE));
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
 * Field handler to provide acess control for the email field
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_user_mail extends views_handler_field {

  // An example of field level access control.
  function access() {
    return user_access('administer users');
  }
}

/**
 * Field handler to provide simple renderer that allows using a themed user link
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_user_name extends views_handler_field_user {
  function render_link($data, $values) {
    if (!empty($this->options['link_to_user'])) {
      $account = new stdClass();
      $account->name = $values->{$this->field_alias};
      $account->uid = $values->{$this->aliases['uid']};
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
      $title = db_result(db_query("SELECT u.name FROM {users} u WHERE u.uid = %d", $this->argument));
    }
    if (empty($title)) {
      return t('No user');
    }

    return check_plain($title);
  }
}

/**
 * Allow replacement of current userid so we can cache these queries
 */
function user_views_query_substitutions($view) {
  global $user;
  return array('***CURRENT_USER***' => intval($user->uid));
}

/**
 * @}
 */
