<?php
// $Id$

/**
 * Implementation of hook_views_default_views().
 */
function search_views_default_views() {
  $views = array();

  $view = new view;
  $view->name = 'backlinks';
  $view->description = t('Displays a list of nodes that link to the node, using the search backlinks table.');
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
  'style_plugin' => 'list',
  'style_options' =>
  array (
    'type' => 'ol',
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
    'title' =>
    array (
      'id' => 'title',
      'table' => 'node',
      'field' => 'title',
      'label' => '',
      'relationship' => 'none',
      'link_to_node' => 1,
    ),
  ),
  'sorts' =>
  array (
  ),
  'arguments' =>
  array (
    'nid' =>
    array (
      'id' => 'nid',
      'table' => 'search_node_links_to',
      'field' => 'nid',
      'default_action' => 'not found',
      'style_plugin' => 'default_summary',
      'style_options' =>
      array (
        'count' => true,
        'override' => false,
        'items_per_page' => 25,
      ),
      'wildcard' => '',
      'wildcard_substitution' => '',
      'title' => t('Pages that link to %1'),
      'default_argument_type' => 'fixed',
      'default_argument' => '',
      'validate_type' => 'node',
      'validate_fail' => 'not found',
      'relationship' => 'none',
      'default_argument_fixed' => '',
      'default_argument_php' => '',
      'validate_argument_node_type' =>
      array (
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
      'value' => 1,
      'group' => 0,
      'exposed' => false,
      'expose' =>
      array (
        'operator' => false,
        'label' => '',
      ),
      'relationship' => 'none',
      'expose_button' =>
      array (
        'button' => 'Expose',
      ),
    ),
  ),
  'items_per_page' => 30,
  'use_pager' => '1',
  'pager_element' => 0,
  'offset' => 0,
  'empty' => t('No backlinks found.'),
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
  'path' => 'node/%/backlinks',
  'menu' =>
  array (
    'type' => 'tab',
    'title' => t('What links here'),
    'weight' => '0',
  ),
  );
  $view->display['page'] = $display;
  $display = new views_display;
  $display->id = 'block';
  $display->display_title = t('Block');
  $display->display_plugin = 'block';
  $display->position = '3';
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
    'items_per_page' => false,
    'offset' => false,
    'use_pager' => false,
    'pager_element' => false,
    'use_more' => false,
    'distinct' => true,
    'link_display' => true,
    'style_plugin' => false,
    'style_options' => false,
    'row_plugin' => false,
    'row_options' => false,
    'relationships' => true,
    'fields' => true,
    'sorts' => true,
    'arguments' => false,
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
    'nid' =>
    array (
      'id' => 'nid',
      'table' => 'search_node_links_to',
      'field' => 'nid',
      'default_action' => 'default',
      'style_plugin' => 'default_summary',
      'style_options' =>
      array (
        'count' => true,
        'override' => false,
        'items_per_page' => 25,
      ),
      'wildcard' => '',
      'wildcard_substitution' => '',
      'title' => t('Displays a list of nodes that link to the node, using the search backlinks table.'),
      'default_argument_type' => 'php',
      'default_argument' => '',
      'validate_type' => 'node',
      'validate_fail' => 'not found',
      'relationship' => 'none',
      'default_argument_fixed' => '',
      'default_argument_php' => 'return ($node = menu_get_object()) ? $node->nid : FALSE;',
      'validate_argument_node_type' =>
      array (
      ),
      'validate_argument_php' => '',
    ),
  ),
  'filters' =>
  array (
  ),
  'items_per_page' => 6,
  'offset' => 0,
  'use_pager' => '0',
  'pager_element' => 0,
  'use_more' => 1,
  'style_plugin' => 'list',
  'style_options' =>
  array (
    'type' => 'ul',
  ),
  'row_plugin' => 'fields',
  'row_options' =>
  array (
  ),
  'block_description' => t('What links here'),
  );
  $view->display['block'] = $display;

  $views[$view->name] = $view;
  return $views;
}
