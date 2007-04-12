<?php

// $Id$
/*
 * This builds a query for the view.
 */
function _views_build_query(&$view, $args = array(), $filters = array()) {
  $query = new _views_query('node', 'nid', !empty($view->use_alias_prefix) ? $view->use_alias_prefix : '');

  // Process static filters and exposed filters
  _views_view_build_filters($query, $view, $filters);

  // Process arguments.
  $view->args = $args; // we'll need these later.

  foreach ($view->argument as $i => $argument) {
    if ($args[$i] != NULL && $args[$i] != '' && $args[$i] != '$arg') {
      _views_view_build_arg($query, $args[$i], $argument);
    }
    else {
      // This is what we do if we expected an arg but we didn't get it.
      switch ($argument['argdefault']) {
        case 1:
          $info['fail'] = TRUE;
          return $info;
        case 3:
        case 4:
          $self_sort = "ASC";
        case 5:
          // the if is a kind of a hack to make this case fall-thru fluid
          if (!$self_sort) {
            $self_sort = "DESC";
        }
        case 6:
          $level = $i;
          _views_build_summary($query, $argument['type'], $argument['options'], $self_sort);
          $summary = true;
          if (!$self_sort) {
            $sort = true;
          }
          break 2; // switch and for
        case 7:
          // to trigger empty text, send back an empty query
          $info['query'] = NULL;
          return $info;
      }
    }
  }

  if (!$summary || $sort) {
    _views_view_build_sorts($query, $view);
    $plugins = _views_get_style_plugins();
    if (($view->page && ($plugins[$view->page_type]['needs_fields'])) ||
      ($view->block && ($plugins[$view->block_type]['needs_fields']))) {
      _views_view_build_fields($query, $view);
    }
  }

  foreach (module_implements('views_query_alter') as $module) {
    $function = $module .'_views_query_alter';
    $results = $function($query, $view, $summary, $level);
  }

  $info['query'] = $query->query();
  $info['countquery'] = $query->query(true);
  $info['summary'] = $summary;
  $info['level'] = $level;
  $info['args'] = $query->where_args;

  return $info;

}

/*
 * Take all the filters in a view and add them to the query object.
 */
function _views_view_build_filters(&$query, &$view, $exposed_filter_values) {
  $filters = _views_get_filters();
  $view->used_filters = array();

  foreach ($view->filter as $i => $filter) {
    $filterinfo = $filters[$filter['field']]; // shortcut
    $field = $filterinfo['field'];
    if (!$field) {
      $fieldbits = explode('.', $filter['field']);
      $field = $fieldbits[1];
    }
    $filterinfo['field'] = $field;

    foreach($view->exposed_filter as $count => $expose) {
      if ($filter['id'] == $expose['id']) {
        $id = $expose['id'];

        if (isset($view->exposed_filter_offset)) {
          $count += $view->exposed_filter_offset;
        }

        if (!$expose['operator'] && $exposed_filter_values[$count]['op']) {
          $filter['operator'] = check_plain($exposed_filter_values[$count]['op']);
          $view->used_filters["op$count"] = $exposed_filter_values[$count]['op'];
        }

        if ($expose['optional']) {
          if ((!isset($exposed_filter_values[$count]['filter']) || $exposed_filter_values[$count]['filter'] == '') && !$expose['is_default']) {
            continue 2; // skip
          }
          if ($exposed_filter_values[$count]['filter'] == '**ALL**' ||
            (is_array($exposed_filter_values[$count]['filter']) && in_array('**ALL**', $exposed_filter_values[$count]['filter']))) {
            $view->used_filters["filter$count"] = $exposed_filter_values[$count]['filter'];
            continue 2; // skip this filter entirely.
          }
        }
        if (isset($exposed_filter_values[$count]['filter']) && $exposed_filter_values[$count]['filter'] != '') {
          $value = $exposed_filter_values[$count]['filter'];
          if ($filterinfo['value-type'] == 'array' && !is_array($value)) {
            $value = array($value);
          }
          $filter['value'] = $value;
          $view->used_filters["filter$count"] = $exposed_filter_values[$count]['filter'];
        }
        break;
      }
    }
    $table = $filterinfo['table'];

    $replace = array('&gt;' => '>', '&lt;' => '<');
    $filter['operator'] = strtr($filter['operator'], $replace);

    if (function_exists($filterinfo['handler'])) {
      $filterinfo['handler']('handler', $filter, $filterinfo, $query);
    }
    else if (is_array($filter['value']) && count($filter['value'])) {
      if ($filter['operator'] == 'OR' || $filter['operator'] == 'NOR') {
        $query->ensure_table($table);
        $where_args = array_merge(array($query->use_alias_prefix . $table, $field), $filter['value']);
        $placeholder = array_fill(0, count($filter['value']), '%s');
        if ($filter['operator'] == 'OR') {
          $query->add_where("%s.%s IN ('". implode("','", $placeholder) ."')", $where_args);
        }
        else {
          $where_args[] = $where_args[0];
          $where_args[] = $where_args[1];
          $query->add_where("(%s.%s NOT IN ('". implode("','", $placeholder) ."') OR %s.%s IS NULL)", $where_args);
        }
      }
      else {
        $howmany = count($filter['value']);
        $high_table = $query->add_table($table, true, $howmany);
        if (!$high_table) { // couldn't add the table
          break;
        }

        $table_num = $high_table - $howmany;
        foreach ($filter['value'] as $item) {
          $table_num++;
          $tn = $query->get_table_name($table, $table_num);
          $query->add_where("%s.%s = '%s'", $tn, $field, $item);
        }
      }
    }
    else {
      $query->ensure_table("$table");
      $query->add_where("%s.%s %s '%s'", $query->use_alias_prefix . $table, $field, $filter['operator'], $filter['value']);
    }
  }
}

/*
 * Add argument data to a view; mostly passes through to handlers.
 */
function _views_view_build_arg(&$query, $arg, $argdata) {
  $arginfo = _views_get_arguments();
  if (!function_exists($arginfo[$argdata['type']]['handler'])) {
    return false;
  }

  // call the handler, unless the arg is the 'wildcard'.
  if ($arg != $argdata['wildcard']) {
    $arginfo[$argdata['type']]['handler']('filter', $query, $argdata, $arg);
  }
}

/*
 * Add sorting information from a view to a query.
 */
function _views_view_build_sorts(&$query, $view) {
  $sorts = _views_get_sorts();

  foreach ($view->sort as $i => $sort) {
    $sortinfo = $sorts[$sort['field']];
    $field = $sortinfo['field'];

    if (!$field) {
      $fieldbits = explode('.', $sort['field']);
      $field = $fieldbits[1];
    }
    $table = $sortinfo['table'];
    if (isset($sortinfo['handler']) && function_exists($sortinfo['handler'])) {
      $sortinfo['field'] = $field;
      $sortinfo['handler']('sort', $query, $sortinfo, $sort);
    }
    else {
      $query->add_orderby($table, $field, $sort['sortorder']);
    }
  }
}

/*
 * Add field information from a view to a query.
 */
function _views_view_build_fields(&$query, &$view) {
  $fields = _views_get_fields();

  foreach ($view->field as $field) {
    // Some fields are not actually fields but require special handlers
    // to display their data.

    $fieldname = $field['fullname']; // combined table & fieldname
    $fieldinfo = $fields[$fieldname]; // laziness + readability

    if (isset($fieldinfo['query_handler']) && function_exists($fieldinfo['query_handler'])) {
      $fieldinfo['query_handler']($field, $fieldinfo, $query);
    }

    if (!$fieldinfo['notafield']) {
      if ($fieldinfo['field']) {
        $query->add_field($fieldinfo['field'], $field['tablename'], $field['queryname']);
      }
      else {
        $query->add_field($field['field'], $field['tablename'], $field['queryname']);
      }
    }
    // special handling
    if (is_array($fieldinfo['addlfields'])) {
      foreach ($fieldinfo['addlfields'] as $name)
        $query->add_field($name, $field['tablename'], "$field[tablename]_$name");
    }
  }
  $plugins = _views_get_style_plugins();
  if (($view->page && $plugins[$view->page_type]['needs_table_header']) ||
      ($view->block && $plugins[$view->block_type]['needs_table_header'])) { // table only
    $view->table_header = _views_construct_header($view, $fields);
    $query->set_header($view->table_header);
  }
}

/*
 * Build a summary if an argument was required and is not present. Mostly
 * this calls handlers.
 */
function _views_build_summary(&$query, $argtype, $option, $self_sort) {
  $arginfo = _views_get_arguments();
  if (!function_exists($arginfo[$argtype]['handler'])) {
    return false;
  }
  $primary_field = $query->fields[0];
  $query->clear_fields();

  $fieldinfo = $arginfo[$argtype]['handler']('summary', $query, $argtype, $option);

  if ($fieldinfo['fieldname']) {
    $query->add_field($fieldinfo[field], '', $fieldinfo[fieldname]);
  }
  $query->add_field("count($primary_field)", '', 'num_nodes');
  $query->add_groupby($fieldinfo['field']);
  $query->set_count_field("DISTINCT($fieldinfo[field])");

  if ($self_sort) {
    $arginfo[$argtype]['handler']('sort', $query, $self_sort);
  }
  $query->no_distinct = TRUE; // prevent DISTINCT from mucking up summaries.
}

// ---------------------------------------------------------------------------
// Query construction object

class _views_query {

  /*
   * Create the basic query object and fill with default values.
   */
  function _views_query($primary_table = 'node', $primary_field = 'nid', $alias_prefix = '') {
    $this->primary_table = $primary_table;
    $this->primary_field = $primary_field;
    $this->joins = array();
    $this->where = array();
    $this->orderby = array();
    $this->groupby = array();
    $this->tables = array();
    $this->where_args = array();
    $this->use_alias_prefix = $alias_prefix;
    // Joins care about order, so we put our tables in a queue to make sure
    // the order is correct.
    $this->tablequeue = array();
    if ($primary_field) {
      $this->fields = array($alias_prefix ."$primary_table.$primary_field");
    }
    $this->count_field = $alias_prefix ."$primary_table.$primary_field";
    $this->header = array();
  }

  /*
   * Add a field to the query table, possibly with an alias. This will
   * automatically call ensure_table to make sure the required table
   * exists, *unless* $table is unset.
   */
  function add_field($field, $table = '$$', $alias = '') {
    if ($table == '$$') {
      // I picked $$ because it's not a legal name for a table and NULL is
      // actually a valid possibility here, and I can't default to a variable.
      $table = $this->primary_table;
    }

    // We check for this specifically because we don't want to add it aliased.
    if ($table == $this->primary_table && $field == $this->primary_field) {
      return;
    }

    if ($table) {
      $this->ensure_table($table);
      $table = $this->use_alias_prefix . $table .".";
    }

    if ($alias) {
      $a = " AS $this->use_alias_prefix$alias";
    }

    if (!in_array("$table$field$a", $this->fields)) {
      $this->fields[] = "$table$field$a";
    }
  }

  /*
   * Set the base field to be distinct.
   */
  function set_distinct($value = TRUE) {
    if (!($this->no_distinct && $value)) {
      $this->distinct = $value;
    }
  }

  /*
   * Remove all fields that may've been added; primarily used for summary
   * mode where we're changing the query because we didn't get data we needed.
   */
  function clear_fields() {
    $this->fields = array();
  }

  /*
   * Set what field the query will count() on for paging.
   */
  function set_count_field($field) {
    $this->count_field = $field;
  }

  /*
   * Add a simple WHERE clause to the query. The caller is responsible for
   * ensuring that all fields are fully qualified (TABLE.FIELD) and that
   * the table already exists in the query.
   */
  function add_where($clause) {
    $args = func_get_args();
    array_shift($args); // ditch $clause

    if (count($args) == 1 && is_array(reset($args))) {
      $args = current($args);
    }
    if (is_array($args)) {
      $this->where[] = $clause;
      $this->where_args = array_merge($this->where_args, $args);
    }
  }

  /*
   * Add multiple an orderby's. Right now I'm not sure why this function
   * is separate from add_orderby above; I don't think it needs to
   * be.
   *
   * NOTE: $alias doesn't work when adding multiple orderbys.
   */
  function add_orderby($table, $field, $order, $alias = '') {
    if (!$alias && $table) {
      $alias = $this->use_alias_prefix . $table;
    }
    elseif ($alias) {
      $alias = $this->use_alias_prefix . $alias;
    }

    if ($table) {
      $this->ensure_table($table);
    }

    if (!is_array($field)) {
      $field = array($field);
    }

    foreach($field as $f) {
      // Only fill out this aliasing if there is a table;
      // otherwise we assume it is a formula.
      if ($table) {
        $as = $alias . '_' . $f;
      }
      else {
        $as = $alias;
      }
      $this->add_field($f, $table, $as);
      $this->orderby[] = "$as $order";
      if ($this->groupby) {
        $this->add_groupby($as);
      }
    }
  }

  /*
   * Add a simple GROUP BY clause to the query. The caller is responsible
   * for ensuring that the fields are fully qualified and the table is properly
   * added.
   */
  function add_groupby($clause) {
    $this->groupby[] = $clause;
  }

  /*
   * This function will ensure a table exists; if it already exists it won't
   * do anything, but if it doesn't it will add the table queue. It will ensure
   * a path leads back to the views_get_title table.
   */
  function ensure_table($table) {
    if ($table == $this->primary_table || $this->tables[$table]) {
      return;
    }

    if ($this->ensure_path($table)) {
      $this->queue_table($table);
    }
  }

  /*
   * This function will add a table to the query.
   *
   * @param $table
   *   The name of the table to add. It needs to exist in the global table
   *   array.
   * @param $ensure_path
   *   If true, it will ensure that a path back to primary_table exists. If false, it
   *   assumes the caller is already sure about this and does not repeat it.
   * @param $howmany
   *   How many instances of the table to add? (Useful for interesting join
   *   combos)
   * @param $joininfo
   *   In some join configurations this table may actually join back through
   *   a different method; this is most likely to be used when tracing
   *   a hierarchy path. (node->parent->parent2->parent3). This parameter
   *   will specify how this table joins if it is not the default.
   */
  function add_table($table, $ensure_path = false, $howmany = 1, $joininfo = NULL) {
    if ($table == $this->primary_table) {
      return;
    }

    if ($ensure_path && !$this->ensure_path($table)) {
      return false;
    }

    $this->tables[$table] += $howmany;

    for ($i = $this->tables[$table] - $howmany + 1; $i <= $this->tables[$table]; $i++) {
      if ($joininfo) {
        $this->joins[$table][$i] = $joininfo;
      }
      $this->tablequeue[] = array('table' => $table, 'num' => $i, 'alias_prefix' => $this->use_alias_prefix);
    }
    return $this->tables[$table];
  }

  /*
   * Add a table to the queue in the specified position. This is a low level
   * operation and should only be performed after all safety checking has been done.
   */
  function queue_table($table) {
    if (!isset($this->tables[$table])) {
      $this->tables[$table] = 1;
    }
    else {
      $this->tables[$table]++;
    }
    $this->tablequeue[] = array('table' => $table, 'num' => $this->tables[$table], 'alias_prefix' => $this->use_alias_prefix);
  }

  /*
   * Set the table header; used for click-sorting because it's needed
   * info to modify the ORDER BY clause.
   */
  function set_header($header) {
    $this->header = $header;
  }

  /*
   * Make sure that the specified table can be properly linked to the views_get_title
   * table in the JOINs. This function uses recursion. If the tables
   * needed to complete the path back to the views_get_title table are not in the
   * query they will be added, but additional copies will NOT be added
   * if the table is already there.
   */
  function ensure_path($table, $traced = array(), $add = array()) {
    if ($table == $this->primary_table) {
      return true;
    }
    $table_data = _views_get_tables();

    $left_table = $table_data[$table]['join']['left']['table'];

    // Does it end at our views_get_title table?
    if ($left_table == $this->primary_table) {
      // We are done! Add our tables and unwind.
      foreach (array_reverse($add) as $table) {
        $this->queue_table($table);
      }
      return true;
    }

    // Have we been this way?
    if (isset($traced[$left_table])) {
      // we looped. Broked.
      return false;
    }

    // Do we have to add this table?
    if (!isset($this->tables[$left_table])) {
      $add[] = $left_table;
    }

    // Keep looking.
    $traced[$left_table] = 1;
    return $this->ensure_path($left_table, $traced, $add);
  }

  /*
   * Return the alias of a given table in the JOIN queries. If a table is in
   * the query multiple times, use this function to figure out what successive
   * iterations of the table will be named.
   *
   * @param $table
   *   The name of the table in the global table array.
   * @param $table_num
   *   The instance number of the table.
   * @param $alias_prefix
   *   An optional prefix for the table alias.
   */
  function get_table_name($table, $table_num, $alias_prefix = null) {
    if (is_null($alias_prefix)) {
      $alias_prefix = $this->use_alias_prefix;
    }
    return ($table_num < 2 ? $alias_prefix . $table : $alias_prefix . $table . $table_num);
  }

  /*
   * Generate a query and a countquery from all of the information supplied
   * to the object.
   *
   * @param $getcount
   *   Provide a countquery if this is true, otherwise provide a normal query.
   */
  function query($getcount = false) {
    $table_data = _views_get_tables();

    // Set distinct
    if (!$this->no_distinct && $this->distinct && count($this->fields)) {
      $field = $this->fields[0];
      $this->fields[0] = "DISTINCT($field)";
      $this->count_field = "DISTINCT($this->count_field)";
      //this is only needed once
      $this->no_distinct = TRUE;
    }

    // Add all the tables to the query via joins. We assume all LEFT joins.
    foreach ($this->tablequeue as $tinfo) {
      $table = $tinfo['table'];

      // The real table name may differ from what we're calling it.
      $table_real = (isset($table_data[$table]['name']) ? $table_data[$table]['name'] : $table);

      $table_num = $tinfo['num'];
      $table_alias = $this->get_table_name($table, $table_num, $tinfo['alias_prefix']);

      $joininfo = (!$this->joins[$table][$table_num] ?
        $table_data[$table]['join'] : $this->joins[$table][$table_num]);

      $left_table_alias = isset($joininfo['left']['alias']) ? $joininfo['left']['alias'] : $tinfo['alias_prefix'];
      $left_table_alias .= $joininfo['left']['table'];

      // the { is a special character which seems to be treated differently
      // in PHP5 than PHP4 so we do this a little oddly.
      $join_type = $joininfo['type'] == 'inner' ? 'INNER' : 'LEFT';
      $joins .= " $join_type JOIN {" . $table_real . "} $table_alias ON " . $left_table_alias .".".
        $joininfo['left']['field'] . " = $table_alias." . $joininfo['right']['field'];

      if (isset($joininfo['extra'])) {
        foreach ($joininfo['extra'] as $field => $value) {
          $joins .= " AND $table_alias.$field";
          if (is_array($value) && count($value)) {
            $joins .= " IN ('". implode("','", $value) ."')";
          }
          else if ($value !== NULL) {
            $joins .= " = '$value'";
          }
        }
      }
    }

    // If it's not a count query, add our fields
    if (!$getcount) {
      $fields = implode(', ', $this->fields);

      // we only add the groupby if we're not counting.
      if ($this->groupby) {
        $groupby = "GROUP BY " . implode(', ', $this->groupby);
      }

      // ok, tablesort_sql is really, really (really) annoying.
      // 1) it insists on adding the ORDER BY clause.
      // 2) You're supposed to give it your stuff as $before, but
      // 3) You have to add the comma and
      // 4) if it doesn't have anything to add, it returns NOTHING.
      // 5) So I'm just going to get what it sends back and chop off
      //    the orderby, cause otherwise my code is too ugly with
      //    various checks.
      if ($this->header) {
        $result = tablesort_sql($this->header);
        if ($result) {
          $this->orderby[] = str_replace('ORDER BY ', '', $result);
        }
      }

      if ($this->orderby) {
        $orderby = "ORDER BY " . implode(', ', $this->orderby);
      }
    }
    else {
      $fields = "count($this->count_field)";
    }

    if ($this->where) {
      $where = "WHERE (" . implode(') AND (', $this->where) . ')';
    }

    $query = "SELECT $fields FROM {" . $this->primary_table . "} $this->primary_table $joins $where $groupby $orderby";

    $replace = array('&gt;' => '>', '&lt;' => '<');
    $query = strtr($query, $replace);

    return $query;
  }
}

/*
 * Support db_query() style argument replacement in a clause.
 */

function _views_replace_args($clause, $args) {
  // cut & pasted directly from db_query.
  if (isset($args[0]) and is_array($args[0])) { // 'All arguments in one array' syntax
    $args = $args[0];
  }
  _db_query_callback($args, TRUE);
  return preg_replace_callback(DB_QUERY_REGEXP, '_db_query_callback', $clause);
}
