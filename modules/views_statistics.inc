<?php
// $Id$

/**
 * This include file implements views functionality on behalf of statistics.module
 */

function statistics_views_tables() {
  $tables['node_counter'] = array(
    'name' => 'node_counter',
    'provider' => 'internal', // won't show up in external list.
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
      'totalcount' => array(
        'name' => t('Node: Total Hits'),
        'sortable' => true,
        'help' => t('This will display the number of times a node has been read.'),
      ),
      'daycount' => array(
        'name' => t('Node: Recent Hits'),
        'sortable' => true,
        'help' => t('This will display the number of times a node has been read recently.'),
      ),
      'timestamp' => array(
        'name' => t('Node: Last Hit Time'),
        'sortable' => true,
        'handler' => views_handler_field_dates(),
        'option' => 'string',
        'help' => t('Display the time the node was last read.'),
      ),
    ),
    'sorts' => array(
      'totalcount' => array(
        'name' => t('Node: Total Hits'),
        'help' => t('This allows you to sort by the number of times a node has been read.'),
      ),
      'daycount' => array(
        'name' => t('Node: Recent Hits'),
        'help' => t('This allows you to sort by the number of times a node has been read recently.'),
      ),
      'timestamp' => array(
        'name' => t('Node: Last Hit Time'),
        'help' => t('This allows you to sort by the time a node was last read.'),
      ),
    ),
  );
  return $tables;
}

function statistics_views_arguments() {

}

function statistics_views_default_views() {
  $view = new stdClass();
  $view->name = 'popular_recent';
  $view->description = t('Nodes sorted by recent page-views');
  $view->access = array (
);
  $view->view_args_php = '';
  $view->page = TRUE;
  $view->page_title = t('recent popular content');
  $view->page_header = '';
  $view->page_header_format = '1';
  $view->page_footer = '';
  $view->page_footer_format = '1';
  $view->page_empty = '';
  $view->page_empty_format = '1';
  $view->page_type = 'table';
  $view->url = 'popular/latest';
  $view->use_pager = TRUE;
  $view->nodes_per_page = '20';
  $view->menu = TRUE;
  $view->menu_title = '';
  $view->menu_tab = TRUE;
  $view->menu_tab_default = TRUE;
  $view->menu_weight = '-1';
  $view->block = TRUE;
  $view->block_title = t('recent popular content');
  $view->block_header = '';
  $view->block_header_format = '1';
  $view->block_footer = '';
  $view->block_footer_format = '1';
  $view->block_empty = '';
  $view->block_empty_format = '1';
  $view->block_type = 'list';
  $view->nodes_per_block = '5';
  $view->block_more = '1';
  $view->block_use_page_header = FALSE;
  $view->block_use_page_footer = FALSE;
  $view->block_use_page_empty = FALSE;
  $view->sort = array (
  );
  $view->argument = array (
  );
  $view->field = array (
    array (
      'tablename' => 'node',
      'field' => 'type',
      'label' => 'type',
      'sortable' => '1',
    ),
    array (
      'tablename' => 'node',
      'field' => 'title',
      'label' => 'title',
      'handler' => 'views_handler_field_nodelink',
      'sortable' => '1',
    ),
    array (
      'tablename' => 'users',
      'field' => 'name',
      'label' => 'author',
      'sortable' => '1',
    ),
    array (
      'tablename' => 'node',
      'field' => 'created',
      'label' => 'created',
      'handler' => 'views_handler_field_date_small',
      'sortable' => '1',
    ),
    array (
      'tablename' => 'node_counter',
      'field' => 'daycount',
      'label' => 'recent hits',
      'sortable' => '1',
      'defaultsort' => 'DESC',
    ),
  );
  $view->filter = array (
    array (
      'tablename' => 'node',
      'field' => 'status',
      'operator' => '=',
      'options' => '',
      'value' => '1',
    ),
  );
  $view->requires = array(node_counter, node, users);
  $views[$view->name] = $view;

  $view = new stdClass();
  $view->name = 'popular_alltime';
  $view->description = t('Nodes sorted by total page-views');
  $view->access = array (
);
  $view->view_args_php = '';
  $view->page = TRUE;
  $view->page_title = t('all-time popular content');
  $view->page_header = '';
  $view->page_header_format = '1';
  $view->page_footer = '';
  $view->page_footer_format = '1';
  $view->page_empty = '';
  $view->page_empty_format = '1';
  $view->page_type = 'table';
  $view->url = 'popular/alltime';
  $view->use_pager = TRUE;
  $view->nodes_per_page = '20';
  $view->menu = TRUE;
  $view->menu_title = '';
  $view->menu_tab = TRUE;
  $view->menu_tab_default = FALSE;
  $view->menu_weight = '1';
  $view->block = TRUE;
  $view->block_title = t('all-time popular content');
  $view->block_header = '';
  $view->block_header_format = '1';
  $view->block_footer = '';
  $view->block_footer_format = '1';
  $view->block_empty = '';
  $view->block_empty_format = '1';
  $view->block_type = 'list';
  $view->nodes_per_block = '5';
  $view->block_more = '1';
  $view->block_use_page_header = FALSE;
  $view->block_use_page_footer = FALSE;
  $view->block_use_page_empty = FALSE;
  $view->sort = array (
  );
  $view->argument = array (
  );
  $view->field = array (
    array (
      'tablename' => 'node',
      'field' => 'type',
      'label' => t('type'),
      'sortable' => '1',
    ),
    array (
      'tablename' => 'node',
      'field' => 'title',
      'label' => t('title'),
      'handler' => 'views_handler_field_nodelink',
      'sortable' => '1',
    ),
    array (
      'tablename' => 'users',
      'field' => 'name',
      'label' => t('author'),
      'sortable' => '1',
    ),
    array (
      'tablename' => 'node',
      'field' => 'created',
      'label' => t('created'),
      'handler' => 'views_handler_field_since',
      'sortable' => '1',
    ),
    array (
      'tablename' => 'node_counter',
      'field' => 'totalcount',
      'label' => t('reads'),
      'sortable' => '1',
      'defaultsort' => 'DESC',
    ),
  );
  $view->filter = array (
    array (
      'tablename' => 'node',
      'field' => 'status',
      'operator' => '=',
      'options' => '',
      'value' => '1',
    ),
  );
  $view->requires = array(node_counter, node, users);
  $views[$view->name] = $view;

  return $views;
}

function theme_views_view_popular_recent($view, $type, $nodes) {
  if ($type == 'block') {
    $fields = _views_get_fields();

    foreach ($nodes as $node) {
      foreach ($view->field as $field) {
        if ($field['fullname'] == 'node.title') {
          $item = _views_handle_field($fields, $field, $node);
        }
        if ($field['fullname'] == 'node_counter.daycount') {
          $item .= ' <span class="popular-content-count">(' . _views_handle_field($fields, $field, $node) . ')</span>';
        }
      }
      $items[] = $item;
    }

    $output = theme('item_list', $items);
    if ($view->block_more && count($nodes) >= $view->nodes_per_block) 
      $output .= theme('views_more', $view->real_url);
    return "<div class='view' id='view-$view->name'>$output</div>\n";
  }
  else {
     return theme('views_view', $view, $type, $nodes);
  }
}

function theme_views_view_popular_alltime($view, $type, $nodes) {
  if ($type == 'block') {
    $fields = _views_get_fields();

    foreach ($nodes as $node) {
      foreach ($view->field as $field) {
        if ($field['fullname'] == 'node.title') {
          $item = _views_handle_field($fields, $field, $node);
        }
        if ($field['fullname'] == 'node_counter.totalcount') {
          $item .= ' <span class="popular-content-count">(' . _views_handle_field($fields, $field, $node) . ')</span>';
        }
      }
      $items[] = $item;
    }

    $output = theme('item_list', $items);
    return "<div class='view' id='view-$view->name'>$output</div>\n";
  }
  else {
     return theme('views_view', $view, $type, $nodes);
  }
}