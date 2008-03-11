<?php
// $Id$
/**
 * @file
 * Provide views data and handlers for statistics.module
 */

/**
 * @defgroup views_statistics_module statistics.module handlers
 *
 * Includes the ability to create views of just the statistics table.
 * @{
 */

/**
 * Implementation of hook_views_data()
 */
function statistics_views_data() {
  // Basic table information.

  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.
  $data['node_counter']['table']['group']  = t('Node statistics');

  // For other base tables, explain how we join
  $data['node_counter']['table']['join'] = array(
    'node' => array(
      'handler' => 'views_join', // this is actually optional
      'arguments' => array('node_counter', 'node', 'nid', 'nid'),
     ),
  );

  // ----------------------------------------------------------------
  // Fields

  // totalcount
  $data['node_counter']['totalcount'] = array(
    'title' => t('Total views'), // The item it appears as on the UI,
    'help' => t('The total number of times the node has been viewed.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
     ),
    'filter' => array(
      'handler' => 'views_handler_filter_gtlt',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // daycount
  $data['node_counter']['daycount'] = array(
    'title' => t('Views today'), // The item it appears as on the UI,
    'help' => t('The total number of times the node has been viewed today.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
     ),
    'filter' => array(
      'handler' => 'views_handler_filter_gtlt',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // timestamp
  $data['node_counter']['timestamp'] = array(
    'title' => t('Most recent view'), // The item it appears as on the UI,
    'help' => t('The most recent time the node has been viewed.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );


  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.
  $data['accesslog']['table']['group']  = t('Access log');

  // Advertise this table as a possible base table
  $data['accesslog']['table']['base'] = array(
    'field' => 'aid',
    'title' => t('Access log'),
    'help' => t('Stores site access information.'),
    'weight' => 10,
  );

  // For other base tables, explain how we join
  $data['accesslog']['table']['join'] = array(
    'users' => array(
      'handler' => 'views_join', // this is actually optional
      'arguments' => array('accesslog', 'users', 'uid', 'uid'),
     ),
  );
  $data['users']['table']['join'] = array(
    'accesslog' => array(
      'handler' => 'views_join', // this is actually optional
      'arguments' => array('users', 'accesslog', 'uid', 'uid'),
     ),
  );


  // ----------------------------------------------------------------
  // Fields

  // session id
  $data['accesslog']['sid'] = array(
    'title' => t('Session ID'), // The item it appears as on the UI,
    'help' => t('Browser session ID of user that visited page.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
     ),
  );

  // title
  $data['accesslog']['title'] = array(
    'title' => t('Page title'), // The item it appears as on the UI,
    'help' => t('Title of page visited.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_accesslog_path',
      'click sortable' => TRUE,
     ),
  );

  // path
  $data['accesslog']['path'] = array(
    'title' => t('Path'), // The item it appears as on the UI,
    'help' => t('Internal path to page visited (relative to Drupal root.)'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_accesslog_path',
      'click sortable' => TRUE,
     ),
  );

  // referrer
  $data['accesslog']['url'] = array(
    'title' => t('Referrer'), // The item it appears as on the UI,
    'help' => t('Referrer URI.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_url',
      'click sortable' => TRUE,
     ),
  );

  // hostname
  $data['accesslog']['hostname'] = array(
    'title' => t('Hostname'), // The item it appears as on the UI,
    'help' => t('Hostname of user that visited the page.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
     ),
  );

  // hostname
  $data['accesslog']['uid'] = array(
    'title' => t('User'), // The item it appears as on the UI,
    'help' => t('The user who visited the site.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_uid',
      'click sortable' => TRUE,
     ),
  );

  // timer
  $data['accesslog']['timer'] = array(
    'title' => t('Timer'), // The item it appears as on the UI,
    'help' => t('Time in milliseconds that the page took to load.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_gtlt',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // timestamp
  $data['accesslog']['timestamp'] = array(
    'title' => t('Timestamp'), // The item it appears as on the UI,
    'help' => t('Timestamp of when the page was visited.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );


  return $data;
}

/**
 * Field handler to provide simple renderer that turns a URL into a clickable link.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_accesslog_path extends views_handler_field {
  /**
   * Override init function to provide generic option to link to node.
   */
  function init(&$view, &$data) {
    parent::init($view, $data);
    if (isset($data['display_as_link']) && $data['field'] != 'path') {
      $this->additional_fields[] = 'path';
    }
  }

  /**
   * Provide link to the page being visited.
   */
  function options_form(&$form, &$form_state) {
    $form['display_as_link'] = array(
      '#title' => t('Display as link'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['display_as_link']),
    );
  }

  function render($values) {
    $title = check_plain($values->{$this->field_alias});
    if (!empty($this->options['display_as_link'])) {
      return l($title, $values->accesslog_path, array('html' => TRUE));
    }
    else {
      return $title;
    }
  }
}


/**
 * Field handler to provide simple renderer that allows using a themed user link
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_uid extends views_handler_field {
  function options(&$options) {
    parent::options($options);
    $options['link_to_user'] = TRUE;
  }

  /**
   * Provide link to node option
   */
  function options_form(&$form) {
    $form['link_to_user'] = array(
      '#title' => t('Link this field to its user'),
      '#type' => 'checkbox',
      '#default_value' => $this->options['link_to_user'],
    );
  }

  /**
   * Called to add the field to a query.
   */
  function query() {
    parent::query();
    if (!empty($this->options['link_to_user'])) {
      $query = $this->view->query;
      $query->add_field('users', 'name');
    }
  }

  function render($values) {
    if (!empty($this->options['link_to_user'])) {
      $account = (object)array('uid' => $values->{$this->field_alias}, 'name' => $values->users_name);
      return theme('username', $account);
    }
    else {
      return $values->{$this->field_alias};
    }
  }
}
