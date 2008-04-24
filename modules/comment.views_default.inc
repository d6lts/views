<?php
// $Id$
/**
 * @file
 * Contains default views on behalf of the comment module.
 */

/**
 * Implementation of hook views_default_views().
 */
function comment_views_default_views() {
  $views = array();
  $view = new view;
  $view->name = 'tracker';
  $view->description = t('Shows all new activity on system.');
  $view->tag = t('default');
  $view->view_php = '';
  $view->base_table = 'node';
  $view->is_cacheable = '0';
  $view->api_version = 2;
  $view->disabled = TRUE;
  $view->display = array();
    $display = new views_display;
    $display->id = 'default';
    $display->display_title = t('Defaults');
    $display->display_plugin = 'default';
    $display->position = '1';
    $display->display_options = array (
    'style_plugin' => 'table',
    'style_options' =>
    array (
      'override' => 1,
      'order' => 'desc',
      'columns' =>
      array (
        'type' => 'type',
        'title' => 'title',
        'name' => 'name',
        'comment_count' => 'comment_count',
        'last_comment_timestamp' => 'last_comment_timestamp',
        'timestamp' => 'title',
        'new_comments' => 'comment_count',
      ),
      'info' =>
      array (
        'type' =>
        array (
          'sortable' => 1,
          'separator' => '',
        ),
        'title' =>
        array (
          'sortable' => 1,
          'separator' => '&nbsp;',
        ),
        'name' =>
        array (
          'sortable' => 1,
          'separator' => '',
        ),
        'comment_count' =>
        array (
          'sortable' => 1,
          'separator' => '<br />',
        ),
        'last_comment_timestamp' =>
        array (
          'sortable' => 1,
          'separator' => '&nbsp;',
        ),
        'timestamp' =>
        array (
          'separator' => '',
        ),
        'new_comments' =>
        array (
          'separator' => '',
        ),
      ),
      'default' => 'last_comment_timestamp',
    ),
    'row_plugin' => 'fields',
    'row_options' =>
    array (
    ),
    'relationships' =>
    array (
    ),
    'fields' =>
    array (
      'type' =>
      array (
        'id' => 'type',
        'table' => 'node',
        'field' => 'type',
        'label' => t('Type'),
      ),
      'title' =>
      array (
        'id' => 'title',
        'table' => 'node',
        'field' => 'title',
        'label' => t('Title'),
        'link_to_node' => TRUE,
      ),
      'name' =>
      array (
        'id' => 'name',
        'table' => 'users',
        'field' => 'name',
        'label' => t('Author'),
        'link_to_user' => true,
      ),
      'comment_count' =>
      array (
        'id' => 'comment_count',
        'table' => 'node_comment_statistics',
        'field' => 'comment_count',
        'label' => t('Replies'),
        'set_precision' => false,
        'precision' => 0,
        'decimal' => '.',
        'separator' => ',',
        'prefix' => '',
        'suffix' => '',
      ),
      'last_comment_timestamp' =>
      array (
        'id' => 'last_comment_timestamp',
        'table' => 'node_comment_statistics',
        'field' => 'last_comment_timestamp',
        'label' => t('Last Post'),
        'date_format' => 'small',
        'custom_date_format' => '',
      ),
      'timestamp' =>
      array (
        'id' => 'timestamp',
        'table' => 'history_user',
        'field' => 'timestamp',
        'label' => '',
        'comments' => 1,
        'relationship' => 'none',
        'link_to_node' => 0,
        'comment' => 1,
      ),
      'new_comments' =>
      array (
        'id' => 'new_comments',
        'table' => 'node',
        'field' => 'new_comments',
        'label' => '',
        'set_precision' => false,
        'precision' => 0,
        'decimal' => '.',
        'separator' => ',',
        'prefix' => '',
        'suffix' => t(' new'),
        'link_to_comment' => 1,
        'no_empty' => 1,
        'relationship' => 'none',
      ),
    ),
    'sorts' =>
    array (
      'last_comment_timestamp' =>
      array (
        'id' => 'last_comment_timestamp',
        'table' => 'node_comment_statistics',
        'field' => 'last_comment_timestamp',
        'order' => 'ASC',
        'granularity' => 'second',
      ),
    ),
    'arguments' =>
    array (
      'uid' =>
      array (
        'id' => 'uid',
        'table' => 'users',
        'field' => 'uid',
        'default_action' => 'ignore',
        'style_plugin' => 'default_summary',
        'style_options' =>
        array (
          'count' => true,
          'override' => false,
          'items_per_page' => 25,
        ),
        'wildcard' => 'all',
        'wildcard_substitution' => t('All'),
        'title' => t('Recent posts for %1'),
        'default_argument_type' => 'fixed',
        'default_argument' => '',
        'validate_type' => 'none',
        'validate_fail' => 'not found',
        'relationship' => 'none',
        'default_argument_fixed' => '',
        'default_argument_php' => '',
        'validate_argument_node_type' =>
        array (
          'album' => 0,
          'artist' => 0,
          'book' => 0,
          'page' => 0,
          'story' => 0,
          'track' => 0,
        ),
        'validate_argument_php' => '',
      ),
    ),
    'filters' =>
    array (
      'status' =>
      array (
        'id' => 'status',
        'table' => 'node',
        'field' => 'status',
        'operator' => '=',
        'value' => '1',
        'group' => 0,
        'exposed' => false,
        'expose' =>
        array (
          'operator' => false,
          'label' => '',
        ),
      ),
    ),
    'items_per_page' => '25',
    'title' => t('Recent posts'),
    'header' => NULL,
    'header_format' => NULL,
    'footer' => NULL,
    'footer_format' => NULL,
    'empty' => NULL,
    'empty_format' => NULL,
    'use_pager' => true,
    'pager_element' => 0,
    'offset' => 0,
    'access' =>
    array (
      'type' => 'none',
      'role' =>
      array (
      ),
      'perm' => '',
    ),
  );
  $view->display['default'] = $display;
    $display = new views_display;
    $display->id = 'page';
    $display->display_title = t('Page');
    $display->display_plugin = 'page';
    $display->position = '2';
    $display->display_options = array (
    'defaults' =>
    array (
      'access' => true,
      'title' => true,
      'header' => true,
      'header_format' => true,
      'header_empty' => true,
      'footer' => true,
      'footer_format' => true,
      'footer_empty' => true,
      'empty' => true,
      'empty_format' => true,
      'use_ajax' => true,
      'items_per_page' => true,
      'offset' => true,
      'use_pager' => true,
      'pager_element' => true,
      'use_more' => true,
      'distinct' => true,
      'link_display' => true,
      'style_plugin' => true,
      'style_options' => true,
      'row_plugin' => true,
      'row_options' => true,
      'relationships' => true,
      'fields' => true,
      'sorts' => true,
      'arguments' => true,
      'filters' => true,
    ),
    'relationships' =>
    array (
    ),
    'fields' =>
    array (
    ),
    'sorts' =>
    array (
    ),
    'arguments' =>
    array (
    ),
    'filters' =>
    array (
    ),
    'path' => 'tracker',
    'menu' =>
    array (
      'type' => 'normal',
      'title' => t('Recent posts'),
    ),
    'tab_options' =>
    array (
      'title' => NULL,
      'weight' => NULL,
    ),
  );
  $view->display['page'] = $display;

  $views[$view->name] = $view;
  return $views;
}
