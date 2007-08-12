<?php
// $Id$
/** 
 * @file view.inc
 * Provides the view object type and associated methods.
 */

class view {
  var $vid = 0;
  var $name = '';
  var $description = '';
  var $display = array();
  var $argument = array();
  var $field = array();
  var $sort = array();
  var $filter = array();
  var $relationship = array();

  var $base_table = 'node';

  var $view_args_php = '';

  // State variables
  var $is_built = FALSE;
  
  var $args = array();

  var $query = NULL;
  var $count_query = NULL;

  function set_args($args) {
    $this->args = $args;
  }

  function set_page_size($page_size) { 
    $this->page_size = $page_size;
  }

  function set_use_pager($use_pager) {
    $this->use_pager = $use_pager;
  }

  function set_offset($offset) { 
    $this->offset = $offset;
  }

  function set_filter_input($filters) {
    $this->filter_input = $filters;
  }

  function set_display($display_id) {
    $this->display = $display_id;
  }

  function build($display_id = NULL) { }
  function render() { }

  function get_title($context) { }
  function get_url() { }
  function is_cacheable() { }

  /**
   * Load a view from the database based upon either vid or name.
   */
  function load($arg) {    
    $where = (is_numeric($arg) ? "vid =  %d" : "name = '%s'");
    $data = db_fetch_object(db_query("SELECT * FROM {views_view} WHERE $where", $arg));
    _views_unpack_schema($this, 'views_view', $data);
    $this->_load_row('display');
    $this->_load_row('argument');
    $this->_load_row('field');
    $this->_load_row('sort');
    $this->_load_row('filter');
    $this->_load_row('relationship');
  }

  /**
   * Load one of our sub tables.
   */
  function _load_row($key) {
    $object_name = "views_$key";
    $table = $object_name . 's';
    $result = db_query("SELECT * FROM {$table} WHERE vid = %d ORDER BY position", $this->vid);

    while ($data = db_fetch_object($result)) {
      $object = new $object_name;
      _views_unpack_schema($object, $table, $data);
      array_push($this->$key, $object);
    }

  }

  /**
   * Save the view to the database. If the view does not already exist, 
   * A vid will be assigned to the view and also returned from this function.
   */
  function save() {
    if (!empty($this->vid)) {
      // remove existing table entries
      db_query("DELETE from {views_displays} WHERE vid = %d", $this->vid);
      db_query("DELETE from {views_arguments} WHERE vid = %d", $this->vid);
      db_query("DELETE from {views_fields} WHERE vid = %d", $this->vid);
      db_query("DELETE from {views_sorts} WHERE vid = %d", $this->vid);
      db_query("DELETE from {views_filters} WHERE vid = %d", $this->vid);
      db_query("DELETE from {views_relationships} WHERE vid = %d", $this->vid);
    }

    _views_save_query('views_view', $this, !empty($this->vid) ? 'vid' : FALSE);
    $this->_save_rows('display');
    $this->_save_rows('argument');
    $this->_save_rows('field');
    $this->_save_rows('sort');
    $this->_save_rows('filter');
    $this->_save_rows('relationship');

    cache_clear_all('views_urls', 'cache_views');
    cache_clear_all(); // clear the page cache as well.
  }

  function _save_rows($key) {
    foreach ($this->$key as $position => $object) {
      $object->position = $position;
      $object->vid = $this->vid;
      _views_save_query("views_" . $key . "s", $object);
    }
  }

  function delete() { 
    if (empty($view->vid)) {
      return;
    }

    db_query("DELETE FROM {views_view} where vid = %d", $view->vid);
    db_query("DELETE FROM {views_displays} where vid = %d", $view->vid);
    db_query("DELETE FROM {views_arguments} where vid = %d", $view->vid);
    db_query("DELETE FROM {views_fields} where vid = %d", $view->vid);
    db_query("DELETE FROM {views_sorts} where vid = %d", $view->vid);
    db_query("DELETE FROM {views_filters} where vid = %d", $view->vid);
    db_query("DELETE FROM {views_relationships} where vid = %d", $view->vid);

    cache_clear_all('views_query:' . $view->name, 'cache_views');
    cache_clear_all(); // In Drupal 5.0 and later this clears the page cache only.
  
  }

  function export() {
    require_once drupal_get_path('module', 'views') . '/includes/export.inc';
    views_export_view($this);
  }
}

class views_argument {
  var $type = '';
  var $default_action = '';
  var $title = '';
  var $wildcard = '';
  var $wildcard_text = '';
  var $summary_format;
  var $options = array();
  var $position = 0;
}

class views_field {
  var $table = '';
  var $field = '';
  var $label = '';
  var $handler = '';
  var $position = 0;
  // Options contains things like: Sortable, default sort, column, etc.
  // Based upon the needs of the output type.
  var $options = array();
}

class views_sort {
  var $table = '';
  var $field = '';
  var $order = '';
  var $options = array();
  var $position = 0;
}

class views_filter {
  var $table = '';
  var $field = '';
  var $group = '';
  var $operator = '';
  var $value = '';
  var $options = '';
  var $exposed = FALSE;
  var $exposed_settings = array();
  var $position = 0;
}

class views_display {
  var $type = '';
  var $output_type = '';
  var $access = '';
  var $title = '';
  var $header = '';
  var $header_format = '';
  var $header_hide_if_empty = '';
  var $footer = '';
  var $footer_format = '';
  var $footer_hide_if_empty = '';
  var $empty = '';
  var $empty_format = '';
  var $use_pager = '';
  var $page_size = '';
  var $url = '';
  var $display_options = array();
  var $output_options = array();
  var $position = 0;

  var $filters_type = ''; // 'table', 'list'
  var $filters_location = ''; // 'view', 'block'
}

class views_relationship {
  var $relationship = '';
  var $link = '';
  var $position = 0;
}


// build an insert/update query based upon schema info.
function _views_save_query($table, &$object, $update = NULL) {
  $schema = drupal_get_schema($table);
  $fields = $defs = $values = $serials = array();

  // Go through our schema and build correlations.
  foreach ($schema['fields'] as $field => $info) {
    if (!isset($object->$field)) {
      if (isset($info['default'])) {
        $object->$field = $info['default'];
      }
      else {
        $object->$field = '';
      }
    }
    // special case -- skip serial types if we are updating.
    if ($info['type'] == 'serial' && $update) {
      continue;
    }
    $fields[] = $field;
    switch ($info['type']) {
      case 'serial':
        $defs[] = '%s';
        $object->$field = 'NULL';
        $serials[] = $field;
        break;
      case 'int':
        $defs[] = '%d';
        break;
      case 'float':
      case 'numeric':
        $defs[] = '%f';
        break;
      default:
        $defs[] = "'%s'";
    }
    if (empty($info['serialize'])) {
      $values[] = $object->$field;
    }
    else {
      $values[] = serialize($object->$field);
    }
  }
  $query = '';
  if (!$update) {
    $query = "INSERT INTO {$table} (" . implode(', ', $fields) . ') VALUES (' . implode(', ', $defs) . ')';
  }
  else {
    $query = '';
    foreach ($fields as $id => $field) {
      if ($query) {
        $query .= ', ';
      }
      $query .= $field . ' = ' . $defs[$id];
    }
    $query = "UPDATE {$table} SET " . $query . " WHERE $update = " . $object->$update;
  }
  db_query($query, $values);

  if ($serials) {
    // get last insert ids and fill them in.
    foreach ($serials as $field) {
      $object->$field = db_last_insert_id($table, $field);
    }
  }
}

function _views_unpack_schema(&$object, $table, $data) {
  $schema = drupal_get_schema($table);
  // Go through our schema and build correlations.

  foreach ($schema['fields'] as $field => $info) {
    $object->$field = empty($info['serialize']) ? $data->$field : unserialize($data->$field);
  }
}
