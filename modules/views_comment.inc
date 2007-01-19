<?php
// $Id$

/**
 * This include file implements views functionality on behalf of comment.module
 */

function comment_views_tables() {
  $tables['comments'] = array(
    'name' => 'comments',
    'provider' => 'internal',
    'join' => array(
      'left' => array(
        'table' => 'node',
        'field' => 'nid'
      ),
      'right' => array(
        'field' => 'nid'
      ),
    ),
    'fields' => array(
      'subject' => array(
        'name' => t('Comment: Subject'),
        'handler' => array(
          'views_handler_field_commentlink'           => t('Normal'),
          'views_handler_field_commentlink_with_mark' => t('With updated mark')
          ),
        'option' => array(
           '#type' => 'select',
           '#options' => array(
             'link' => 'As link',
             'nolink' => 'Without link'
            ),
        ),
        'addlfields' => array('cid'),
        'sortable' => TRUE,
      ),
      'comment' => array(
        'name' => t('Comment: Comment'),
        'handler' => 'views_handler_field_comment',
        'addlfields' => array('format'),
        'help' => t('Display the content of a comment.'),
      ),
      'cid' => array(
        'name' => t('Comment: Comment ID'),
        'sortable' => TRUE,
        'help' => t('Display the CID of a comment.'),
      ),
      'timestamp' => array(
        'name' => t('Comment: Created Time'),
        'sortable' => TRUE,
        'handler' => views_handler_field_dates(),
        'option' => 'string',
        'help' => t('Display the post time of the comment.'),
      ),
      'name' => array(
        'name' => t('Comment: Author Name'),
        'handler' => 'views_handler_field_username',
        'sortable' => TRUE,
        'uid' => 'uid',
        'addlfields' => array('uid'),
        'help' => t('This will display the author of the node.'),
      ),
    ),
    'filters' => array(
      'status' => array(
        'name' => t('Comment: Pending approval'),
        'operator' => array('=' => t('Equals')),
        'list' => 'views_handler_operator_yesno',
        'list-type' => 'select',
        ),
    ),
    'sorts' => array(
      'timestamp' => array(
        'name' => t('Comment: Created Time'),
        'handler' => 'views_handler_sort_date',
        'option' => views_handler_sort_date_options(),
        'help' => t('Sort by the created time of comments.'),
      ),
    ),
  );

  $tables['node_comment_statistics'] = array(
    'name' => 'node_comment_statistics',
    'provider' => 'internal',
    'join' => array(
      'left' => array(
        'table' => 'node',
        'field' => 'nid'
      ),
      'right' => array(
        'field' => 'nid'
      ),
    ),
    'fields' => array(
      'last_comment_timestamp' => array(
        'name' => t('Comment: Last Comment Time'),
        'sortable' => true,
        'handler' => views_handler_field_dates(),
        'option' => 'string',
        'help' => t('This will display the last comment time.'),
       ),
      'last_comment_name' => array(
        'name' => t('Comment: Last Comment Author'),
        'query_handler' => 'views_query_handler_field_last_comment_name',
        'handler' => 'views_handler_field_last_comment_name',
        'sortable' => true,
        'uid' => 'last_comment_uid',
        'addlfields' => array('last_comment_name', 'last_comment_uid'),
        'help' => t('This will display the name of the last user to comment on the post.'),
      ),
      'comment_count' => array(
        'name' => t('Comment: Count'),
        'sortable' => true,
        'handler' => array(
          'views_handler_field_int'         => t('Normal'),
          'views_handler_comments_with_new' => t('With New Count')
        ),
        'help' => t('This will display the comment count.'),
      ),
      'last_changed' => array(
        'name' => t('Comment: Last Changed Time'),
        'query_handler' => 'views_query_handler_field_last_changed',
        'sortable' => TRUE,
        'handler' => views_handler_field_dates(),
        'option' => 'string',
        'notafield' => TRUE,
        'help' => t('This will display the time of the last comment or node edit.'),
      ),
    ),
    'filters' => array(
      'comment_count' => array(
        'name' => t('Comment: Comment Count'),
        'operator' => 'views_handler_operator_gtlt',
        'option' => 'integer',
        'help' => t('This filter allows you to filter by the amount of comments.'),
      ),
      'last_comment_timestamp' => array(
        'name' => t('Comment: Last Comment Time'),
        'operator' => 'views_handler_operator_gtlt',
        'value' => views_handler_filter_date_value_form(),
        'handler' => 'views_handler_filter_timestamp',
        'option' => 'string',
        'help' => t('This filter allows nodes to be filtered by the timestamp for the last comment or node edit. The "Value" can either be a date in the format: CCYY-MM-DD HH:MM:SS or the word "NOW" to use the current time. You may enter a positive or negative number in the "Option" field that will represent the amount of seconds that will be added or substracted to the time; this is most useful when combined with "NOW". If you have the jscalendar module from jstools installed, you can use a popup date picker here.'),
      ),
      'last_changed' => array(
        'name' => t('Comment: Last Changed Time'),
        'operator' => 'views_handler_operator_gtlt',
        'value' => views_handler_filter_date_value_form(),
        'handler' => 'views_handler_filter_last_changed',
        'option' => 'string',
        'help' => t('This filter allows nodes to be filtered by the timestamp for the last comment or node edit. The "Value" can either be a date in the format: CCYY-MM-DD HH:MM:SS or the word "NOW" to use the current time. You may enter a positive or negative number in the "Option" field that will represent the amount of seconds that will be added or substracted to the time; this is most useful when combined with "NOW". If you have the jscalendar module from jstools installed, you can use a popup date picker here.'),
      ),

    ),
    'sorts' => array(
      'last_comment_timestamp' => array(
        'name' => t('Comment: Last Comment Date'),
        'handler' => 'views_handler_sort_date',
        'option' => views_handler_sort_date_options(),
        'help' => t('This will allow you to sort by the date of the most recent comment on a node.')
      ),
      'comment_count' => array(
        'name' => t('Comment: Comment Count'),
        'help' => t('This filter allows you to sort by the number of comments.'),
      ),
      'last_changed' => array(
        'name' => t('Comment: Last Changed'),
        'handler' => 'views_handler_sort_last_changed',
        'option' => views_handler_sort_date_options(),
        'help' => t('Sort based on the most recent comment or edit for the node.'),
      ),
    ),
  );
  return $tables;
}

function views_query_handler_field_last_changed($field, $fieldinfo, &$query) {
  $query->add_field('GREATEST(node.changed, node_comment_statistics.last_comment_timestamp)', '', $field['tablename'] . '_last_changed');
}

function views_handler_filter_last_changed($op, $filter, $filterinfo, &$query) {
  $value = $filter['value'] == 'now' ? "***CURRENT_TIME***" : strtotime($filter['value']);

  $table = $filterinfo['table'];
  $column = $table . '_' . $filterinfo['field'];
  $field = "GREATEST(node.changed, node_comment_statistics.last_comment_timestamp)";
  $query->ensure_table($table);
  $query->add_where("%s %s %s + %d", $field, $filter['operator'], $value, $filter['options']);
}


function views_handler_sort_last_changed($action, &$query, $sortinfo, $sort) {
  $query->ensure_table('node_comment_statistics');
  $query->orderby[] = 'GREATEST(node.changed, node_comment_statistics.last_comment_timestamp) ' . $sort['sortorder'];  
}

function views_query_handler_field_last_comment_name($field, $fieldinfo, &$query) {
  $num = $query->add_table('users', false, 1, array(
    'left' => array(
      'table' => $field['tablename'],
      'field' => 'last_comment_uid'
    ),
    'right' => array(
      'field' => 'uid'
    ),
  ));
  $query->add_field('name', $query->get_table_name('users', $num), $field['tablename'] . '_name');
}

function views_handler_field_last_comment_name($fieldinfo, $fielddata, $value, $data) {
  $obj = new stdClass();
  $obj->name = $value;
  $uidfield = $fielddata['tablename'] . "_"  . $fieldinfo['uid'];
  $obj->uid = $data->$uidfield;
  if ($obj->uid != 0) {
    $fieldname = $fielddata['tablename'] . '_name';
    $obj->name = $data->$fieldname;
  }
  return theme('username', $obj);
}

/*
 * Format a field as a link to a comment.
 */
function views_handler_field_commentlink($fieldinfo, $fielddata, $value, $data) {
  if ($fielddata['options'] == 'nolink') {
    return check_plain($value);
  }
  return l($value, "node/$data->nid", NULL, NULL, "comment-$data->comments_cid");
}

/*
 * Format a field as a number of comments, plus the number of unread comments.
 */
function views_handler_comments_with_new($fieldinfo, $fielddata, $value, $data) {
  $comments = intval($value);
  if ($comments && $new = comment_num_new($data->nid)) {
    $comments .= '<br />';
    $comments .= l(t('@num new', array('@num' => $new)), "node/$data->nid", NULL, NULL, 'new');
  }
  return $comments;
}

/*
 * Format a field as a link to a 'mark', stating whether or not the comment has
 * updated since it was last viewed by the user.
 */
function views_handler_field_commentlink_with_mark($fieldinfo, $fielddata, $value, $data) {
  if ($fielddata['options'] == 'nolink') {
    $link = check_plain($value);
  }
  else {
    $link = l($value, "node/$data->nid", NULL, NULL, "comment-$data->comments_cid");
  }
  return $link .' '. theme('mark', node_mark($data->nid, $data->comments_timestamp));
}

/*
 * Format a field as a comment
 */
function views_handler_field_comment($fieldinfo, $fielddata, $value, $data) {
  $obj = new stdClass();
  $obj->comment = check_markup($data->comments_comment, $data->comments_format, FALSE);
  comment_invoke_comment($obj, 'view'); 
  return $obj->comment;
}

/*
function comment_views_arguments() {

}
*/
function comment_views_default_views() {
  $view = new stdClass();
  $view->name = 'comments_recent';
  $view->description = t('Display recent comments block');
  $view->access = array (
);
  $view->view_args_php = '';
  $view->block = TRUE;
  $view->block_title = t('Recent comments');
  $view->block_header = "";
  $view->block_header_format = '1';
  $view->block_footer = "";
  $view->block_footer_format = '1';
  $view->block_empty = "";
  $view->block_empty_format = '1';
  $view->block_type = 'list';
  $view->nodes_per_block = '10';
  $view->block_more = '0';
  $view->block_use_page_header = FALSE;
  $view->block_use_page_footer = FALSE;
  $view->block_use_page_empty = FALSE;
  $view->sort = array (
    array (
      'tablename' => 'comments',
      'field' => 'timestamp',
      'sortorder' => 'DESC',
      'options' => '',
    ),
  );
  $view->argument = array (
  );
  $view->field = array (
    array (
      'tablename' => 'comments',
      'field' => 'subject',
      'label' => '',
      'handler' => 'views_handler_field_commentlink',
      'options' => 'link',
    ),
    array (
      'tablename' => 'comments',
      'field' => 'name',
      'label' => '',
    ),
    array (
      'tablename' => 'comments',
      'field' => 'timestamp',
      'label' => '',
      'handler' => 'views_handler_field_date_small',
    ),
  );
  $view->filter = array (
    array (
      'tablename' => 'node_comment_statistics',
      'field' => 'comment_count',
      'operator' => '>',
      'options' => '',
      'value' => '1',
    ),
    array (
      'tablename' => 'comments',
      'field' => 'status',
      'operator' => '=',
      'options' => '',
      'value' => '0',
    ),
  );
  $view->exposed_filter = array (
  );
  $view->requires = array(comments, node_comment_statistics);
  $views[$view->name] = $view;

  return $views;
}

