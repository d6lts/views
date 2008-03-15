<?php
function node_views_default_views() {
  $views = array();

  $view = new view;
  $view->name = 'frontpage';
  $view->description = t('The basic front page view.');
  $view->tag = '';
  $view->view_php = '';
  $view->base_table = 'node';
  $view->is_cacheable = '0';
  $view->api_version = 2;
  $view->disabled = FALSE; // Edit this to true to make a default view disabled initially
  $view->display = array();
    $display = new views_display;
    $display->id = 'default';
    $display->display_title = t('Defaults');
    $display->display_plugin = 'default';
    $display->position = '1';
    $display->display_options = array (
    'style_plugin' => 'default',
    'style_options' =>
    array (
    ),
    'row_plugin' => 'node',
    'row_options' =>
    array (
      'teaser' => true,
    ),
    'relationships' =>
    array (
    ),
    'fields' =>
    array (
    ),
    'sorts' =>
    array (
      'sticky' =>
      array (
        'id' => 'sticky',
        'table' => 'node',
        'field' => 'sticky',
        'order' => 'ASC',
      ),
      'created' =>
      array (
        'id' => 'created',
        'table' => 'node',
        'field' => 'created',
        'order' => 'ASC',
      ),
    ),
    'arguments' =>
    array (
    ),
    'filters' =>
    array (
      'promote' =>
      array (
        'id' => 'promote',
        'table' => 'node',
        'field' => 'promote',
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
    'items_per_page' => 10,
    'use_pager' => TRUE,
    'pager_element' => 0,
    'title' => '',
    'header' => '',
    'header_format' => '1',
    'footer' => '',
    'footer_format' => '1',
    'empty' => '',
    'empty_format' => '1',
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
      'items_per_page' => true,
      'offset' => true,
      'use_pager' => true,
      'pager_element' => true,
      'link_display' => true,
      'php_arg_code' => true,
      'exposed_options' => true,
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
    'path' => 'frontpage',
  );
  $view->display['page'] = $display;

  $views[$view->name] = $view;
  return $views;
}
