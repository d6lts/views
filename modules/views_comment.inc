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
        'handler' => array(
          'views_handler_field_date_small' => t('As Short Date'),
          'views_handler_field_date'       => t('As Medium Date'),
          'views_handler_field_date_large' => t('As Long Date'),
          'views_handler_field_since'      => t('As Time Ago')
        ),
        'help' => t('This will display the last comment time.'),
       ),
      'last_comment_name' => array(
        'name' => t('Comment: Last Comment Author'),
        'handler' => 'views_handler_field_username',
        'sortable' => true,
        'uid' => 'last_comment_uid',
        'addlfields' => array('last_comment_uid'),
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
    'sorts' => array(
      'last_comment_timestamp' => array(
        'name' => t('Comment: Last Comment Date'),
        'help' => t('This will allow you to sort by the date of the most recent comment on a node.')
      )
    )
  );
  return $tables;
}

/*
function comment_views_arguments() {

}

function comment_views_default_views() {

}
*/
