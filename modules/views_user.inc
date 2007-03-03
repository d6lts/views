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
      'type' => 'inner',
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
        'handler' => 'views_handler_filter_role_custom',
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
    'username' => array(
      'name' => t('User: Username is Author'),
      'handler' => t('views_handler_arg_username'),
      'help' => t('The Username argument allows users to file to a nodes authored by the specified username.'),
    ),
  );
  return $arguments;
}


function user_views_default_views() {
  $view = new stdClass();
  $view->name = 'tracker';
  $view->requires = array('node_comment_statistics');
  $view->description = t('Shows all new activity on system.');
  $view->page = true;
  $view->url = 'tracker';
  $view->page_title = t('recent posts');
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
      'label' => t('Type'),
    ),
    array (
      'tablename' => 'node',
      'field' => 'title',
      'label' => t('Title'),
      'handler' => 'views_handler_field_nodelink_with_mark',
    ),
    array (
      'tablename' => 'users',
      'field' => 'name',
      'label' => t('Author'),
    ),
    array (
      'tablename' => 'node_comment_statistics',
      'field' => 'comment_count',
      'label' => t('Replies'),
      'handler' => 'views_handler_comments_with_new',
    ),
    array (
      'tablename' => 'node_comment_statistics',
      'field' => 'last_comment_timestamp',
      'label' => t('Last Post'),
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
      'title' => t('recent posts for %1'),
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

/**
 * Allow replacement of current userid so we can cache these queries
 */
function user_views_query_substitutions($view) {
  global $user;
  return array('***CURRENT_USER***' => intval($user->uid));
}

/*
 * Format as a field as a username. Needs to also have a 'uid' available,
 * specified in the field info.
 */
function views_handler_field_username($fieldinfo, $fielddata, $value, $data) {
  $obj = new stdClass();
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
    return theme('user_picture', $account);
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
      return check_plain($user->name);
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
      return check_plain($user->name);
  }
}

function views_handler_arg_username($op, &$query, $argtype, $arg = '') {
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
      $name = check_plain($arg);
      $table_data = _views_get_tables();
      $joininfo = $table_data['users']['join'];
      $query->add_table('users', false, 1, $joininfo);
      $query->add_where("users.name = '%s'", $name);
      break;
    case 'link':
      $name = ($query->name ? $query->name : variable_get('anonymous', 'Anonymous'));
      return l($name, "$arg/" . check_plain($name));
    case 'title':
      if (!$query) {
        return variable_get('anonymous', 'Anonymous');
      } 
      $user = db_fetch_object(db_query("SELECT name FROM {users} WHERE name = '%s'", $query));
      return check_plain($user->name);
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

/*
 * Create a list of roles.
 */
function views_handler_filter_role() {
  $rids = array();
  $result = db_query("SELECT r.rid, r.name FROM {role} r ORDER BY r.name");
  while ($obj = db_fetch_object($result)) {
    $rids[$obj->rid] = $obj->name;
  }
  return $rids;
}

/*
 * Custom filter for roles
 */
function views_handler_filter_role_custom($op, $filter, $filterinfo, &$query) {
  $table = $filterinfo['table'];
  $field = $filterinfo['field'];

  //filter for role authenticated users
  if (in_array(DRUPAL_AUTHENTICATED_RID, $filter['value'])) {
    if ($filter['operator'] == 'OR') {
      //filter for anonymous users too -> return all
      if (in_array(DRUPAL_ANONYMOUS_RID, $filter['value'])) {
        return;
      }
      $query->ensure_table($table);
      $query->add_where("(%s.%s != %d OR %s.%s IS NULL)", $table, $field, DRUPAL_ANONYMOUS_RID, $table, $field);
      return;
    }
    else if ($filter['operator'] == 'NOR') {
      //filter for anonymous users too?
      if (in_array(DRUPAL_ANONYMOUS_RID, $filter['value'])) {
        $query->ensure_table($table);
        $query->add_where("%s.%s = '0'", $table, $field); //this will get no results
        return;
      }
      else {
        $query->ensure_table($table);
        $query->add_where("%s.%s = %d", $table, $field, DRUPAL_ANONYMOUS_RID);
        return;
      }
    }
    else if ($filter['operator'] == 'AND' && !in_array(DRUPAL_ANONYMOUS_RID, $filter['value'])) {
      $key = array_search(DRUPAL_AUTHENTICATED_RID, $filter['value']);
      unset($filter['value'][$key]);
      //if there are other roles left the default filter handling is ok, if not
      //we need to exclude anonymous users
      if (count($filter['value']) == 0) {
        $query->ensure_table($table);
        $query->add_where("%s.%s != %d", $table, $field, DRUPAL_ANONYMOUS_RID);
        return;
      }
    }
  }
  if ($filter['value']) {
    views_handler_filter_default($op, $filter, $filterinfo, $query);
  }
}