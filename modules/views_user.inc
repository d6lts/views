<?php
// $Id$

/**
 * This include file implements views functionality on behalf of user.module
 */

function user_views_tables() {
  $tables['users'] = array(
    'name' => 'users',
    'provider' => 'internal', // won't show up in external list.
    'join' => array(
      'left' => array(
        'table' => 'node',
        'field' => 'uid'
      ),
      'right' => array(
        'field' => 'uid'
      ),
    ),
    'fields' => array(
      'name' => array(
        'name' => t('Node: Author Name'),
        'handler' => 'views_handler_field_username',
        'sortable' => true,
        'uid' => 'uid',
        'addlfields' => array('uid'),
        'help' => t('This will display the author of the node.'),
      ),
      'uid' => array(
        'name' => t('User: Author Picture'),
        'handler' => 'views_handler_field_userpic',
        'sortable' => false,
        'notafield' => true,
        'help' => t('Display the user picture of the author.'),
      ),
    ),
    'sorts' => array(
      'name' => array(
        'name' => t('Node: Author Name'),
        'help' => t('This allows you to sort alphabetically by author.'),
      )
    ),
    'filters' => array(
      'uid' => array(
        'name' => t('Node: Author Name'),
        'operator' => 'views_handler_operator_or',
        'list' => 'views_handler_filter_username',
        'value-type' => 'array',
        'help' => t('This allows you to filter by a particular user. You might not find this useful if you have a lot of users.'),
      ),
    )
  );

  $tables['users_roles'] = array(
    'name' => 'users_roles',
    'provider' => 'internal', // won't show up in external list.
    'join' => array(
      'left' => array(
        'table' => 'node',
        'field' => 'uid'
      ),
      'right' => array(
        'field' => 'uid'
      ),
    ),
    'filters' => array(
      'rid' => array(
        'name' => t('Role: Author Role'),
        'operator' => 'views_handler_operator_andor',
        'list' => 'views_handler_filter_role',
        'value-type' => 'array',
        'help' => t('Include the node only if the author is a member of the selected role.'),
      ),
    )
  );
  return $tables;
}

function user_views_arguments() {
  $arguments = array(
    'uid' => array(
      'name' => t('User: UID is Author'),
      'handler' => 'views_handler_arg_uid',
      'help' => t('The User ID argument allows users to filter to nodes authored by the specified user ID.'),
    ),
    'uidtouch' => array(
      'name' => t('User: UID Authored or Commented'),
      'handler' => 'views_handler_arg_uid_touch',
      'help' => t('The User ID argument allows users to filter a to nodes authored or commented on the specified user ID.'),
    ),
  );
  return $arguments;
}


function user_views_default_views() {
  $view = new stdClass();
  $view->name = 'tracker';
  $view->requires = array('node_comment_statistics');
  $view->description = 'Shows all new activity on system.';
  $view->page = true;
  $view->url = 'tracker';
  $view->page_title = 'recent posts for %1';
  $view->page_type = 'table';
  $view->use_pager = true;
  $view->nodes_per_page = '25';

  $view->menu = true;
  $view->menu_title = 'recent posts';
  $view->block = false;
  $view->field = array (
    array (
      'tablename' => 'node',
      'field' => 'type',
      'label' => 'Type',
    ),
    array (
      'tablename' => 'node',
      'field' => 'title',
      'label' => 'Title',
      'handler' => 'views_handler_field_nodelink_with_mark',
    ),
    array (
      'tablename' => 'users',
      'field' => 'name',
      'label' => 'Author',
    ),
    array (
      'tablename' => 'node_comment_statistics',
      'field' => 'comment_count',
      'label' => 'Replies',
      'handler' => 'views_handler_comments_with_new',
    ),
    array (
      'tablename' => 'node_comment_statistics',
      'field' => 'last_comment_timestamp',
      'label' => 'Last Post',
      'handler' => 'views_handler_field_since',
    ),
  );
  $view->sort = array (
    array (
      'tablename' => 'node_comment_statistics',
      'field' => 'last_comment_timestamp',
      'sortorder' => 'DESC',
    ),
  );
  $view->argument = array (
    array (
      'type' => 'uid',
      'argdefault' => '2',
      'title' => 'recent posts for all users',
    ),
  );
  $view->filter = array (
    array (
      'tablename' => 'node',
      'field' => 'status',
      'operator' => '=',
      'value' => '1',
    ),
  );

  $views[$view->name] = $view;

  return $views;
}

/*
 * Format as a field as a username. Needs to also have a 'uid' available,
 * specified in the field info.
 */
function views_handler_field_username($fieldinfo, $fielddata, $value, $data) {
  $obj->name = $value;
  $uidfield = $fielddata['tablename'] . "_"  . $fieldinfo['uid'];
  $obj->uid = $data->$uidfield;
  return theme('username', $obj);
}

/*
 * Format as a field as a username. Needs to also have a 'uid' available,
 * specified in the field info.
 */
function views_handler_field_userpic($fieldinfo, $fielddata, $value, $data) {
  $account = user_load(array('uid' => $value));
  if ($account !== FALSE) {
    return theme_user_picture($account);
  }
}

function views_handler_arg_uid($op, &$query, $argtype, $arg = '') {
  switch($op) {
    case 'summary':
      $query->add_table('users', true);
      $query->add_field('name', 'users');
      $query->add_field('uid', 'users');
      $fieldinfo['field'] = "users.name";
      return $fieldinfo;
      break;
    case 'sort':
      $query->add_orderby('users', 'name', $argtype);
      break;
    case 'filter':
      $uid = intval($arg);
      $query->add_where("node.uid = $uid");
      break;
    case 'link':
      $name = ($query->name ? $query->name : variable_get('anonymous', 'Anonymous'));
      return l($name, "$arg/" . intval($query->uid));
    case 'title':
      if (!$query) {
        return variable_get('anonymous', 'Anonymous');
      } 
      $user = db_fetch_object(db_query("SELECT name FROM {users} WHERE uid = '%d'", $query));
      return $user->name;
  }
}

function views_handler_arg_uid_touch($op, &$query, $argtype, $arg = '') {
  switch($op) {
    case 'summary':
      $query->add_table('users', true);
      $query->add_field('name', 'users');
      $query->add_field('uid', 'users');
      $fieldinfo['field'] = "users.name";
      return $fieldinfo;
      break;
    case 'sort':
      $query->add_orderby('users', 'name', $argtype);
      break;
    case 'filter':
      $uid = intval($arg);
      $table_data = _views_get_tables();
      $joininfo = $table_data['comments']['join'];
      $joininfo['extra'] = array('uid' => $uid);
      $tblnum = $query->add_table("comments", false, 1, $joininfo);
      $table = $query->get_table_name('comments', $tblnum);

      $query->set_distinct(); // once you do this, there's just one node.
      $query->add_where("node.uid = '%s' OR $table.uid = '%s'", $uid, $uid);
      break;
    case 'link':
      $name = ($query->name ? $query->name : variable_get('anonymous', 'Anonymous'));
      return l($name, "$arg/" . intval($query->uid));
    case 'title':
      if (!$query) {
        return variable_get('anonymous', 'Anonymous');
      } 
      $user = db_fetch_object(db_query("SELECT name FROM {users} WHERE uid = '%d'", $query));
      return $user->name;
  }
}

/*
 * Create a list of usernames.
 * I worry this will really, really suck for large sites.
 * But then, why would large sites create a filter for one particular user?
 * Better to go with roles.
 */
function views_handler_filter_username() {
  $uids = array();
  $result = db_query("SELECT u.uid, u.name FROM {users} u WHERE uid != 0 ORDER BY u.name");
  while ($obj = db_fetch_object($result)) {
    $uids[$obj->uid] = $obj->name;
  }
  return $uids;
}

/*
 * Simple array for anonymous..
 */
function views_handler_filter_useranon() {
  return array("0" => variable_get('anonymous', 'Anonymous'));
}

/*
 * Simple array for current user.
 */
function views_handler_filter_usercurrent() {
  return array('***CURRENT_USER***' => t('Currently Logged In User'));
}

/*
 * Custom filter for current user.
 */
function views_handler_filter_usercurrent_custom($op, $filter, $filterinfo, &$query) {
  global $user;
  $query->ensure_table('users');
  $query->add_where("users.uid $filter[operator] '$user->uid'");
}

/*
 * Add special info to see if a user either authored OR commented on a post
 */
function views_handler_filter_uid_touched($op, $filter, $filterinfo, &$query) {
  if (!module_exist('comment')) { // not much to do in this case
    $query->add_where("node.uid = '%s'", $filter['value']);
    return;
  }

  $table_data = _views_get_tables();
  $joininfo = $table_data['comments']['join'];
  $joininfo['extra'] = array('uid' => $filter['value']);
  $tblnum = $query->add_table('comments', false, 1, $joininfo);
  $table = $query->get_table_name('comments', $tblnum);

  $query->set_distinct(); // once you do this, there's just one node.
  $query->add_where("node.uid = '%s' OR $table.uid = '%s'", $filter['value'], $filter['value']);
}
