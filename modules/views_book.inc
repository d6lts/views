<?php
// $Id$

/**
 * This include file implements views functionality on behalf of book.module
 */

function book_views_tables() {
  $tables['book'] = array(
    'name' => 'book',
    'provider' => 'internal',
    'join' => array(
      'left' => array(
        'table' => 'node',
        'field' => 'nid'
      ),
      'right' => array(
        'field' => 'nid'
      )
    ),
    "filters" => array(
      'parent' => array(
         'name' => t('Book: Parent Node'),
         'list' => 'views_handler_filter_book_parent_zero',
         'operator' => 'views_handler_operator_andor',
         'help' => t('This allows you to filter books based on parent node.'),
      ),
    ),
    'sorts' => array(
      'weight' => array(
        'name' => t('Book: Weight'),
        'handler' => 'views_handler_sort_book_weight',
        'help' => t('This will sort the view by book weight, if that is applicable.'),
      ),
    ),
  );
  $tables['book_parent_node'] = array(
    'name' => 'node',
    'provider' => 'internal',
    'join' => array(
      'left' => array(
        'table' => 'node',
        'field' => 'nid'
      ),
      'right' => array(
        'field' => 'nid'
      )
    ),
    'fields' => array(
      'title' => array(
        'name' => t('Book: Parent Title'),
        'handler' => array(
          'views_handler_field_book_parent_title'    => t('As Link'),
          'views_handler_field_book_parent_title_nl' => t('Without Link'),
        ),
        'addlfields' => array('nid'),
        'help' => t('Display the title of the parent node'),
      ),
    ),
  );
  
  return $tables;
}

function book_views_arguments() {
  $arguments = array(
    'book_parent' => array(
      'name' => t("Book: Parent Node ID"), 
      'handler' => "views_handler_arg_book_parent",
    ),
  );
  return $arguments;
}

function book_views_default_views() {

}

function views_handler_arg_book_parent($op, & $query, $argtype, $arg = '') {
  switch ($op)
  {
    case 'summary' :
      $query->ensure_table("book");
      $query->add_field("parent", "book");
      $query->add_field("title", "node");
      $query->add_field("nid");
      $fieldinfo['field'] = "book.parent";
      return $fieldinfo;
      break;
    case 'sort':
      $query->add_orderby('book', 'weight', $argtype);
      $query->add_orderby('node', 'title', $argtype);
      break;
    case 'filter' :
      $query->ensure_table("book");
      $query->add_where("book.parent = '$arg'");
      $query->add_where("book.vid = node.vid");
      break;
    case 'link' :
      return l($query->title, "$arg/$query->nid");
    case 'title' :
      if ($query)
      {
        $term = db_fetch_object(db_query("SELECT title FROM {node} WHERE nid = '%d'", $query));
        return $term->title;
      }
  }
}

/*
 * Format a field as a link to the book parent node
 */
function views_handler_field_book_parent_title($fieldinfo, $fielddata, $value, $data) {
  return l($value, "node/$data->book_parent_node_nid");
}

function views_handler_field_book_parent_title_nl($fieldinfo, $fielddata, $value, $data) {
  return check_plain($value);
}

function views_handler_filter_book_parent_zero() {
  $parents = array();
  $result = db_query("SELECT DISTINCT parent FROM {book} ORDER BY parent");
  while ($obj = db_fetch_object($result)) {
    $parents[$obj->parent] = "$obj->parent";
  }
  return $parents;
}

