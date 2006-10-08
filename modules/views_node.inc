<?php
// $Id$

/**
 * This include file implements views functionality on behalf of node.module
 */

function node_views_tables() {
  $tables['node'] = array(
    'name' => 'node',
    'provider' => 'internal', // won't show up in external list.
    'fields' => array(
      'title' => array(
        'name' => t('Node: Title'),
        'handler' => array(
          'views_handler_field_nodelink'           => t('Normal'),
          'views_handler_field_nodelink_with_mark' => t('With updated mark')
        ),
        'option' => array(
           '#type' => 'select',
           '#options' => array(
             'link' => 'As link',
             'nolink' => 'Without link'
            ),
        ),
        'sortable' => true,
        'addlfields' => array('changed'),
        'help' => t('Display the title of the node.'),
      ),
      'nid' => array(
        'name' => t('Node: Node ID'),
        'sortable' => true,
        'help' => t('Display the NID of a node.'),
      ),
      'created' => array(
        'name' => t('Node: Created Time'),
        'sortable' => true,
        'handler' => views_handler_field_dates(),
        'option' => 'string',
        'help' => t('Display the post time of the node.'),
      ),
      'changed' => array(
        'name' => t('Node: Updated Time'),
        'sortable' => true,
        'handler' => views_handler_field_dates(),
        'option' => 'string',
        'help' => t('Display the last time the node was updated.'),
      ),
      'type' => array(
        'name' => t('Node: Type'),
        'handler' => 'views_handler_nodetype',
        'sortable' => true,
        'help' => t("The Node Type field will display the type of a node (for example, 'blog entry', 'forum post', 'story', etc)"),
      ),
      'link' => array(
        'name' => t('Node: Link to node'),
        'handler' => 'views_handler_field_node_link',
        'sortable' => false,
        'option' => 'string',
        'notafield' => 'true',
        'help' => t("This will create a link to the node; fill the option field with the text for the link. If you want titles that link to the node, use Node: Title instead."),
      ),
      'body' => array(
        'name' => t('Node: Body'),
        'handler' => array(
          'views_handler_field_body'   => t('Full Text'),
          'views_handler_field_teaser' => t('Teaser')
        ),
        'addlfields' => array('nid'),
        'notafield' => TRUE,
        'help' => t('Display the Main Content.'),
      ),
    ),
    'sorts' => array(
      'nid' => array(
        'name' => t('Node: ID'),
        'help' => t('Sort by the database ID of the node.'),
       ),
      'created' => array(
        'name' => t('Node: Created Time'),
        'help' => t('Sort by the submission date of the node.'),
       ),
      'changed' => array(
        'name' => t('Node: Last Updated Time'),
        'help' => t('Sort by the last update date of the node.'),
      ),
      'sticky' => array(
        'name' => t('Node: Sticky'),
        'help' => t('Sort by whether or not the node is sticky. Choose descending to put sticky nodes at the top.'),
      ),
      'title' => array(
        'name' => t('Node: Title'),
        'help' => t('Sort by the node title, alphabetically'),
      ),
      'random' => array(
        'name' => t('Random'),
        'handler' => 'views_handler_sort_random',
        'help' => t('By choosing random, nodes will be ordered completely randomly. This is a good way to choose X random nodes from a group of nodes.'),
      ),
    ),
    'filters' => array(
      'status' => array(
        'name' => t('Node: Published'),
        'operator' => array('=' => t('Equals')),
        'list' => 'views_handler_operator_yesno',
        'list-type' => 'select',
        'help' => t('Filter by whether or not the node is published. This is recommended for most Views!'),
      ),
      'promote' => array(
        'name' => t('Node: Front Page'),
        'operator' => array('=' => t('Equals')),
        'list' => 'views_handler_operator_yesno',
        'list-type' => 'select',
        'help' => t('Filter by whether or not the node has been promoted to Front Page.'),
      ),
      'sticky' => array(
        'name' => t('Node: Sticky'),
        'operator' => array('=' => t('Equals')),
        'list' => 'views_handler_operator_yesno',
        'list-type' => 'select',
        'help' => t('Filter by whether or not the node is set sticky.'),
      ),
      'moderate' => array(
        'name' => t('Node: Moderated'),
        'operator' => array('=' => t('Equals')),
        'list' => 'views_handler_operator_yesno',
        'list-type' => 'select',
        'help' => t('Filter by whether or not the node is moderated.'),
      ),
      'type' => array(
        'name' => t('Node: Type'),
        'list' => 'views_handler_filter_nodetype',
        'list-type' => 'list',
        'operator' => 'views_handler_operator_or',
        'value-type' => 'array',
        'help' => t('Include or exclude nodes of the selected types.'),
      ),
      'anon' => array(
        'field' => 'uid',
        'name' => t('Node: Author is Anonymous'),
        'operator' => 'views_handler_operator_eqneq',
        'list' => 'views_handler_filter_useranon',
        'value-type' => 'array',
        'help' => t('This allows you to filter by whether or not the node author is anonymous.'),
      ),
      'currentuid' => array(
        'field' => 'uid',
        'name' => t('Node: Author is Current User'),
        'operator' => 'views_handler_operator_eqneq',
        'list' => 'views_handler_filter_usercurrent',
        'list-type' => 'select',
        'help' => t('This allows you to filter by whether or not the node was authored by the logged in user of the view.'),
      ),
      'currentuidtouched' => array(
        'field' => 'uid',
        'name' => t('Node: Current User Authored or Commented'),
        'operator' => array('=' => 'touched by'),
        'list' => 'views_handler_filter_usercurrent',
        'list-type' => 'select',
        'handler' => 'views_handler_filter_uid_touched',
        'help' => t('This allows you to filter by whether or not the logged in user authored or commented on the node.'),
      ),
      'distinct' => array(
        'name' => t('Node: Distinct'),
        'operator' => array('=' => 'is'),
        'list' => array('distinct' => 'distinct'),
        'handler' => 'views_handler_filter_distinct',
        'value-type' => 'array',
        'help' => t('This filter ensures that each node may only be listed once, even if it matches multiple criteria. Use this if multiple taxonomy matches return duplicated nodes.'),
      ),
      'title' => array(
        'name' => t('Node: Title'),
        'operator' => 'views_handler_operator_like',
        'handler' => 'views_handler_filter_like',
        'help' => t('This filter allows nodes to be filtered by their title.'),
      ),
      'created' => array(
        'name' => t('Node: Created Time'),
        'operator' => 'views_handler_operator_gtlt',
        'value' => views_handler_filter_date_value_form(),
        'handler' => 'views_handler_filter_timestamp',
        'option' => 'string',
        'help' => t('This filter allows nodes to be filtered by their creation date. Enter dates in the format: CCYY-MM-DD HH:MM:SS. Enter \'now\' to use the current time. You may enter a delta (in seconds) to the option that will be added to the time; this is most useful when combined with now. If you have the jscalendar module from jstools installed, you can use a popup date picker here.'),
      ),
      'changed' => array(
        'name' => t('Node: Updated Time'),
        'operator' => 'views_handler_operator_gtlt',
        'value' => views_handler_filter_date_value_form(),
        'handler' => 'views_handler_filter_timestamp',
        'option' => 'string',
        'help' => t('This filter allows nodes to be filtered by their creation date. Enter dates in the format: CCYY-MM-DD HH:MM:SS. Enter \'now\' to use the current time. You may enter a delta (in seconds) to the option that will be added to the time; this is most useful when combined with now. If you have the jscalendar module from jstools installed, you can use a popup date picker here.'),
      ),
      'body' => array(
        'name' => t('Node: Body'),
        'operator' => 'views_handler_operator_like',
        'handler' => 'views_handler_filter_body',
        'help' => t('This filter allows nodes to be filtered by their body.'),
      ),
    ),
  );

  // By default history table assumes current user.
  $tables['history'] = array(
    'name' => 'history',
    'provider' => 'internal',
    'join' => array(
      'left' => array(
        'table' => 'node',
        'field' => 'nid'
      ),
      'right' => array(
        'field' => 'nid'
      ),
      'extra' => array(
        'uid' => '***CURRENT_USER***'
      )
    ),
    'filters' => array(
      'timestamp' => array(
        'name' => t('Node: Has New Content'),
        'operator' => array('Has'),
        'list' => array('New Content'),
        'handler' => 'views_handler_filter_isnew',
        'value-type' => 'array',
        'help' => t('Including this filter will reduce the node set to nodes that have been updated or have new content since the user last read the node, as well as unread nodes.'),
      ),
    ),
  );

  return $tables;
}

function node_views_arguments() {
  $arguments = array(
    'nodetype' => array(
      'name' => t('Node: Type'),
      'handler' => 'views_handler_arg_nodetype',
      'help' => t('The node type argument allows users to filter a view by specifying the type of node.'),
    ),
    'nodeletter' => array(
      'name' => t('Node: Title'),
      'handler' => 'views_handler_arg_nodeletter',
      'option' => 'string',
      'help' => t('The argument will filter by the node title. For this argument, set the option to the number of characters, using 0 for full term; use 1 for an A/B/C style glossary.'),
    ),
    'year' => array(
      'name' => t('Node: Posted Year'),
      'handler' => 'views_handler_arg_year',
      'help' => t('This argument allows users to filter by what year the node was posted, in the form of CCYY.'),
    ),
    'month' => array(
      'name' => t('Node: Posted Month'),
      'handler' => 'views_handler_arg_month',
      'help' => t("Months are specified by the numbers 1-12. Since this argument does not specify a year, it is recommended that it usually follow a 'Year' argument."),
    ),
    'week' => array(
      'name' => t('Node: Posted Week'),
      'handler' => 'views_handler_arg_week',
      'help' => t("This allows the user to filter a view by the week number from 1-52. It is recommended this argument follow a 'Year' argument."),
    ),
    'monthyear' => array(
      'name' => t('Node: Posted Month + Year'),
      'handler' => 'views_handler_arg_monthyear',
      'help' => t('This argument combines Month and Year into a single argument, specified in the form CCYYMM.'),
    ),
    'fulldate' => array(
      'name' => t('Node: Posted Full Date'),
      'handler' => 'views_handler_arg_fulldate',
      'help' => t('This argument is a complete date in the form of CCYYMMDD.'),
    ),
    'nid' => array(
      'name' => t('Node: ID'),
      'handler' => 'views_handler_arg_nid',
      'help' => t('This argument is a single Node ID.'),
    ),
    // renamed from 'feed' so things don't DIE when you have viewfeed.module still
    // enabled
    'node_feed' => array(
      'name' => t('Node: Feed Selector'),
      'handler' => 'views_handler_arg_node_feed',
      'help' => t('This argument allows pluggable "feed" selectors. If using views_rss module, "feed" will turn the view into an RSS feed. Other modules may provide their own feeds.'),
    ),
  );
  return $arguments;
}


function node_views_default_views() {

  $view = new stdClass();
  $view->name = 'frontpage';
  $view->description = t('The basic front page view.');
  $view->page = true;
  $view->url = 'frontpage';
  $view->page_title = '';
  $view->page_type = 'teaser';
  $view->use_pager = true;
  $view->nodes_per_page = variable_get('default_nodes_main', 10);
  $view->block = false;
  $view->menu = false;
  $view->breadcrumb_no_home = true;
  $view->sort = array (
    array (
      'tablename' => 'node',
      'field' => 'sticky',
      'sortorder' => 'DESC',
      'options' => '',
    ),
    array (
      'tablename' => 'node',
      'field' => 'created',
      'sortorder' => 'DESC',
      'options' => '',
    ),
  );
  $view->argument = array (
    array (
      'type' => 'node_feed',
      'argdefault' => '2',
      'title' => '',
      'options' => '',
      'wildcard' => '',
      'wildcard_substitution' => '',
    ),
  );
  $view->field = array (
  );
  $view->filter = array (
    array (
      'tablename' => 'node',
      'field' => 'promote',
      'operator' => '=',
      'options' => '',
      'value' => '1',
    ),
    array (
      'tablename' => 'node',
      'field' => 'status',
      'operator' => '=',
      'options' => '',
      'value' => '1',
    ),
  );
  $views[$view->name] = $view;
  return $views;
}

/*
 * Format a field as a link to a node.
 */
function views_handler_field_nodelink($fieldinfo, $fielddata, $value, $data) {
  if ($fielddata['options'] == 'nolink') {
    return check_plain($value);
  }
  return l($value, "node/$data->nid");
}

/*
 * Not to be confused with views_handler_field_nodelink above. This formats
 * the option field as a link to the node.
 */
function views_handler_field_node_link($fieldinfo, $fielddata, $value, $data) {
  return l($fielddata['options'], "node/$data->nid");
}


/*
 * Format a field as a link to a 'mark', stating whether or not the node has
 * updated since it was last viewed by the user.
 */
function views_handler_field_nodelink_with_mark($fieldinfo, $fielddata, $value, $data) {
  if ($fielddata['options'] == 'nolink') {
    $link = check_plain($value);
  }
  else {
    $link = l($value, "node/$data->nid");
  }
  return $link .' '. theme('mark', node_mark($data->nid, $data->node_changed));
}

/*
 * Format a field as a node type.
 */
function views_handler_nodetype($fieldinfo, $fielddata, $value, $data) {
  return node_get_name($value);
}

/*
 * Format a field as a number of comments, plus the number of unread comments.
 */
function views_handler_comments_with_new($fieldinfo, $fielddata, $value, $data) {
  $comments = intval($value);
  if (module_exist('comment') && $comments && $new = comment_num_new($data->nid)) {
    $comments .= '<br />';
    $comments .= l(t('%num new', array('%num' => $new)), "node/$data->nid", NULL, NULL, 'new');
  }
  return $comments;
}

/*
 * Format a field as the Body of a node.
 */
function views_handler_field_body($fieldinfo, $fielddata, $value, $data) {
  $node = node_load($data->nid);

  if ($fielddata['handler'] == 'views_handler_field_body') {
    $teaser = FALSE;
  }
  else {
    $teaser = TRUE;
  }

  $node->body = str_replace('<!--break-->', '', $node->body);

  // The 'view' hook can be implemented to overwrite the default function
  // to display nodes.
  if (node_hook($node, 'view')) {
    node_invoke($node, 'view', $teaser, TRUE);
  }
  else {
    $node = node_prepare($node, $teaser);
  }
  // Allow modules to change $node->body before viewing.
  node_invoke_nodeapi($node, 'view', $teaser, TRUE);

  return $teaser ? $node->teaser : $node->body;
}

/*
 * Format a field as the Teaser of a node.
 */
function views_handler_field_teaser($fieldinfo, $fielddata, $value, $data) {
  return views_handler_field_body($fieldinfo, $fielddata, $value, $data);
}

/*
 * Handle the node type argument.
 */
function views_handler_arg_nodetype($op, &$query, $argtype, $arg = '') {
  switch($op) {
    case 'summary':
      $query->add_field("type");
      $query->add_groupby("node.type");
      $fieldinfo['field'] = "node.type";
      return $fieldinfo;
      break;
    case 'sort':
      $query->add_orderby('node', 'type', $argtype);
      break;
    case 'filter':
      $where = db_escape_string($arg);
      $query->add_where("node.type = '%s'", $where);
      break;
    case 'link':
      return l(node_get_name($query->type), "$arg/$query->type");
    case 'title':
      $title = node_get_name($query);
      return $title ? $title : check_plain($query);
  }
}

function views_handler_arg_nodeletter($op, &$query, $argtype, $arg = '') {
  switch($op) {
    case 'summary':
      $len = intval($arg);
      $fieldinfo['field'] = ($len <= 0 ? "node.title" : $fieldinfo['field'] = "LEFT(node.title, $len)");
      $fieldinfo['fieldname'] = 'letter';
      return $fieldinfo;
      break;
    case 'sort':
      $query->add_orderby('node', 'title', $argtype);
      break;
    case 'filter':
      $len = intval($argtype['options']);
      if ($len <= 0) {
        $query->add_where("node.title = '%s'", $arg);
      } else {
        $query->add_where("LEFT(node.title, $len) = '%s'", $arg);
      }
      break;
    case 'link':
      return l(strtoupper($query->letter), "$arg/$query->letter");
    case 'title':
      return check_plain(strtoupper($query));
  }
}

function views_handler_arg_year($op, &$query, $argtype, $arg = '') {
  $timezone = _views_get_timezone();

  switch($op) {
    case 'summary':
      $fieldinfo['field'] = "YEAR(FROM_UNIXTIME(node.created+$timezone))";
      $fieldinfo['fieldname'] = 'year';
      $query->add_field('created');
      return $fieldinfo;
      break;
    case 'sort':
      $query->add_orderby(NULL, 'year', $argtype);
      break;
    case 'filter':
      $year = intval($arg);
      $query->add_where("YEAR(FROM_UNIXTIME(node.created+$timezone)) = $year");
      break;
    case 'link':
      return l($query->year, "$arg/$query->year");
    case 'title':
      return check_plain($query);
  }
}
function views_handler_arg_month($op, &$query, $argtype, $arg = '') {
  $timezone = _views_get_timezone();

  switch($op) {
    case 'summary':
      $fieldinfo['field'] = "MONTH(FROM_UNIXTIME(node.created+$timezone))";
      $fieldinfo['fieldname'] = 'name';
      $query->add_field('created');
      return $fieldinfo;
      break;
    case 'sort':
      $query->add_orderby(NULL, 'name', $argtype);
      break;
    case 'filter':
      $month = intval($arg);
      $query->add_where("MONTH(FROM_UNIXTIME(node.created+$timezone)) = $month");
      break;
    case 'link':
      return l(format_date($query->created, 'custom', 'F'), "$arg/$query->name");
    case 'title':
      $month = str_pad($query, 2, '0', STR_PAD_LEFT);
      return format_date(strtotime("2005{$month}15"), 'custom', 'F', 0);
  }
}
function views_handler_arg_week($op, &$query, $argtype, $arg = '') {
  $timezone = _views_get_timezone();

  switch($op) {
    case 'summary':
      // The 3 makes the week 1-53, the first week of the year has at least 3 days
      $fieldinfo['field'] = "WEEK(FROM_UNIXTIME(node.created+$timezone), 3)";
      $fieldinfo['fieldname'] = "name";
      $query->add_field('created');
      return $fieldinfo;
      break;
    case 'sort':
      $query->add_orderby(NULL, 'name', $argtype);
      break;
    case 'filter':
      // The 3 makes the week 1-53, the first week of the year has at least 3 days
      $week = intval($arg);
      $query->add_where("WEEK(FROM_UNIXTIME(node.created+$timezone), 3) = $week");
      break;
    case 'link':
      return l("Week $query->name", "$arg/$query->name");
    case 'title':
      return check_plain($query);
  }
}
function views_handler_arg_monthyear($op, &$query, $argtype, $arg = '') {
  $timezone = _views_get_timezone();

  switch($op) {
    case 'summary':
      $fieldinfo['field'] = "DATE_FORMAT(FROM_UNIXTIME(node.created+$timezone), '%Y%m')";
      $fieldinfo['fieldname'] = 'name';
      $query->add_field('created');
      return $fieldinfo;
      break;
    case 'sort':
      $query->add_orderby(NULL, "name", $argtype);
      break;
    case 'filter':
      $query->add_where("DATE_FORMAT(FROM_UNIXTIME(node.created+$timezone), '%Y%m') = '%s'", $arg);
      break;
    case 'link':
      return l(format_date($query->created, 'custom', 'F, Y'), "$arg/$query->name");
    case 'title':
      return format_date(strtotime("${query}01"), 'custom', 'F, Y', 0);
  }
}
function views_handler_arg_fulldate($op, &$query, $argtype, $arg = '') {
  $timezone = _views_get_timezone();

  switch($op) {
    case 'summary':
      $fieldinfo['field'] = "DATE_FORMAT(FROM_UNIXTIME(node.created+$timezone), '%Y%m%%d')";
      $fieldinfo['fieldname'] = 'name';
      $query->add_field('created');
      return $fieldinfo;
      break;
    case 'sort':
      $query->add_orderby(NULL, 'name', $argtype);
      break;
    case 'filter':
      $query->add_where("DATE_FORMAT(FROM_UNIXTIME(node.created+$timezone), '%Y%m%%d') = '%s'", $arg);
      break;
    case 'link':
      return l(format_date($query->created, 'custom', 'F j, Y'), "$arg/$query->name");
    case 'title':
      return format_date(strtotime($query), 'custom', 'F j, Y');
  }
}

function views_handler_arg_nid($op, &$query, $argtype, $arg = '') {
  switch($op) {
    case 'summary':
      $query->add_field('nid');
      $query->add_field("title");
      $fieldinfo['field'] = 'node.nid';
      return $fieldinfo;
    case 'sort':
      // do nothing here.
      break;
    case 'filter':
      $args = _views_break_phrase($arg);
      if ($args[0] == 'and') {
        foreach ($args[1] as $arg) {
          $query->add_where("node.nid = %d", $arg);
        }
      }
      else {
        $query->add_where("node.nid IN (%s)", implode(',', $args[1]));
      }
      break;
    case 'link':
      return l($query->title, "$arg/$query->nid");
    case 'title':
      list($type, $info) = _views_break_phrase($query);
      if (!$info) {
        return t('Untitled');
      }
      $nids = implode(',', $info); // only does numbers so safe

      $result = db_query("SELECT title FROM {node} WHERE nid IN (%s)", $nids);
      while ($node = db_fetch_object($result)) {
        $title .= ($title ? ($type == 'or' ? ' + ' : ', ') : '') . check_plain($node->title);
      }

  }
}

/*
 * Custom filter for new content.
 */
function views_handler_filter_isnew($op, $filter, $filterinfo, &$query) {
  global $user;
  if (!$user || !$user->uid) {
    return;
  }

  // Hey, Drupal kills old history, so nodes that haven't been updated
  // since NODE_NEW_LIMIT are bzzzzzzzt outta here!

  $limit = NODE_NEW_LIMIT;

  $query->ensure_table('history');
  if (module_exist('comment')) {
    $query->ensure_table('node_comment_statistics');
    $clause = "OR node_comment_statistics.last_comment_timestamp > $limit";
    $clause2 = "OR history.timestamp < node_comment_statistics.last_comment_timestamp";
  }

  // NULL means a history record doesn't exist. That's clearly new content.
  // Unless it's very very old content. Everything in the query is already
  // type safe cause none of it is coming from outside here.
  $query->add_where("(history.timestamp IS NULL AND (node.changed > $limit $clause)) OR history.timestamp < node.changed $clause2");
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
 * Create a list of node types.
 */
function views_handler_filter_nodetype($op) {
  $nodes = array();
  foreach (node_get_types() as $type => $name) {
    $nodes[$type] = $name;
  }
  return $nodes;
}

/*
 * Set a query to be distinct as a filter handler
 */
function views_handler_filter_distinct($op, $filter, $filterinfo, &$query) {
  $query->set_distinct();
}

/*
 * Filter by body
 */
function views_handler_filter_body($op, $filter, $filterinfo, &$query) {
  $filterinfo['table'] = 'node_revisions';
  $joininfo = array(
    'left' => array(
      'table' => 'node',
      'field' => 'vid',
      ),
    'right' => array(
      'field' => 'vid',
    ),
  );
  $query->add_table($filterinfo['table'], FALSE, 1, $joininfo);
  views_handler_filter_like($op, $filter, $filterinfo, $query);
}

function views_handler_sort_random($action, &$query, $sortinfo, $sort) {
  switch ($GLOBALS['db_type']) {
    case 'mysql':
    case 'mysqli':
      $query->add_orderby('', "rand()", "ASC");
      break;
    case 'pgsql':
      $query->add_orderby('', "random()", "ASC");
      break;
  }
}

function views_handler_arg_node_feed($op, &$query, $argtype, $arg = '') {
  switch($op) {
    case 'summary':
    case 'sort':
    case 'link':
    case 'title':
      break;
    case 'filter':
      // Can't use node_invoke_all because we're using a reference to
      // $view.
      foreach (module_implements('views_feed_argument') as $name) {
        $function = $name .'_views_feed_argument';
        $function('argument', $GLOBALS['current_view'], $arg);
      }
  }
}

function node_views_post_view($view, $items, $output) {
  foreach ($view->argument as $id => $argument) {
    if ($argument['type'] == 'node_feed') {
      $feed = $id;
      break;
    }
  }

  if ($feed !== NULL) {
    $additions = module_invoke_all('views_feed_argument', 'post_view', $view, $argument['type']);
    return (implode(' ', $additions));
  }
}

/**
 * helper function -- this function builds a URL for a given feed.
 * It defaults to the built in feed selector, but the 3rd arg can
 * be used to set it up for custom selectors too.
 */
function views_post_view_make_url($view, $feed_id, $arg) {
  // assemble the URL
  $args = array();
  foreach ($view->argument as $id => $argdata) {
    if (isset($view->args[$id])) {
      $args[] = $view->args[$id];
    } else {
      if ($argdata['id'] == $feed_id) {
        $args[] = $arg;
      }
      else if ($argdata['argdefault'] != 1) {
        $args[] = '*';
      }
    }
  }
  
  return views_get_url($view, $args);
}

/**
 * helper function -- this function builds a URL for a given feed.
 * It defaults to the built in feed selector, but the 3rd arg can
 * be used to set it up for custom selectors too.
 */
function views_post_view_make_args($view, $feed_id, $arg) {
  // assemble the URL
  $args = array();
  foreach ($view->argument as $id => $argdata) {
    if (isset($view->args[$id])) {
      $args[] = $view->args[$id];
    } else {
      if ($argdata['id'] == $feed_id) {
        $args[] = $arg;
      }
      else if ($argdata['argdefault'] != 1) {
        $args[] = '*';
      }
    }
  }
  
  return $args;
}
