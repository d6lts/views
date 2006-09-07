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
        'help' => t('This filter allows nodes to be filtered by the last comment timestamp. Enter dates in the format: CCYY-MM-DD HH:MM:SS. Enter \'now\' to use the current time. You may enter a delta (in seconds) to the option that will be added to the time; this is most useful when combined with now. If you have the jscalendar module from jstools installed, you can use a popup date picker here.'),
      ),
    ),
    'sorts' => array(
      'last_comment_timestamp' => array(
        'name' => t('Comment: Last Comment Date'),
        'help' => t('This will allow you to sort by the date of the most recent comment on a node.')
      ),
      'comment_count' => array(
        'name' => t('Comment: Comment Count'),
        'help' => t('This filter allows you to sort by the number of comments.'),
      ),
    ),
  );
  return $tables;
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
function comment_views_arguments() {

}

function comment_views_default_views() {

}
*/
