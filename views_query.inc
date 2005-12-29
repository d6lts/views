<?php

/*
 * This builds a query for the view.
 */
function _views_build_query(&$view, $args = array()) {
  $query = new _views_query();

  // Process static filters
  _views_view_build_filters($query, $view);
  
  // Process arguments.
  $view->args = $args; // we'll need these later.

  foreach ($view->argument as $i => $argument) {
    if ($args[$i] != NULL && $args[$i] != '') {
      _views_view_build_arg($query, $args[$i], $argument);
    }
    else { 
      // This is what we do if we expected an arg but we didn't get it.
      switch ($argument['argdefault']) {
        case 1:
          drupal_not_found();
          exit;
        case 3:
          $level = $i;
          _views_build_summary($query, $argument['type'], $argument['options']);
          $summary = true;
          break 2; // switch and for
      } 
    } 
  }

  if (!$summary) {
    _views_view_build_sorts($query, $view);
    if ($view->type == 1 || $view->type == 2) { // list (1) or table (2)
      $fields = _views_get_fields(); 

      foreach($view->field as $field) {
        // Some fields are not actually fields but require special handlers
        // to display their data.
        if (!$field['notafield'])
          $query->add_field($field['field'], $field['tablename'], $field['queryname']);
        // special handling
        if (is_array($fields[$field['fullname']]['addlfields'])) {
          foreach($fields[$field['fullname']]['addlfields'] as $name) 
            $query->add_field($name, $field['tablename'], "$field[tablename]_$name");
        }
      }
      if ($view->type == 2) { // table only
        $view->table_header = _views_construct_header($view, $fields);
        $query->set_header($view->table_header);
      }
    }
  }

  $info['query'] = $query->query();
  $info['countquery'] = $query->query(true);
  $info['summary'] = $summary;
  $info['level'] = $level;

  return $info;

}

/*
 * Take all the filters in a view and add them to the query object.
 */
function _views_view_build_filters(&$query, $view) {
  $filters = _views_get_filters();

  foreach ($view->filter as $i => $filter) {
    $filterinfo = $filters[$filter['field']]; // shortcut
    $field = $filterinfo['field'];
    if (!$field) {
      $fieldbits = explode('.', $filter['field']);
      $field = $fieldbits[1];
    }

    $table = $filterinfo['table'];

    if (function_exists($filterinfo['handler'])) {
      $filterinfo['handler']('handler', $filter, $filterinfo, $query);
    }
    else if (is_array($filter['value'])) {
      if ($filter['operator'] == 'OR' || $filter['operator'] == 'NOR') {
        $not = '';
        if ($filter['operator'] == 'NOR') {
          $not = "NOT";
        }
        $query->ensure_table($table);
        $query->add_where("$table.$field $not IN ('" . implode("','", $filter['value']) . "')");
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
          $query->add_where("$tn.$field = '$item'");
        }
      }
    }
    else {
      $query->ensure_table("$table");
      $query->add_where("$table.$field $filter[operator] '$filter[value]'");
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

  // call the handler
  $arginfo[$argdata['type']]['handler']('filter', $query, $argdata, $arg);
  
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
      $sortinfo['handler']('sort', $query, $sortinfo, $sort);
    }
    else {
      $query->add_orderby_multi($table, $field, $sort['sortorder']);
    }
  }
}

/*
 * Build a summary if an argument was required and is not present. Mostly
 * this calls handlers.
 */
function _views_build_summary(&$query, $argtype, $option) {

  $arginfo = _views_get_arguments();
  if (!function_exists($arginfo[$argtype]['handler'])) {
    return false;
  }

  $query->clear_fields();

  $fieldinfo = $arginfo[$argtype]['handler']('summary', $query, $argtype, $option);

  if ($fieldinfo['fieldname']) {
    $query->add_field("$fieldinfo[field] AS $fieldinfo[fieldname]", '');
  }
  $query->add_field("count(n.nid) as num_nodes", '');
  $query->add_groupby("$fieldinfo[field]");
  $query->set_count_field("DISTINCT($fieldinfo[field])");
}

// ---------------------------------------------------------------------------
// Query construction object

class _views_query {

  /*
   * Create the basic query object and fill with default values.
   */
  function _views_query() {
    $this->where = "";
    $this->orderby = "";
    $this->groupby = "";
    $this->tables = array();
    // Joins care about order, so we put our tables in a queue to make sure
    // the order is correct.
    $this->tablequeue = array();
    $this->field = array("n.nid");
    $this->count_field = "*";
    $this->header = array();
  }

  /*
   * Add a field to the query table, possibly with an alias. This will 
   * automatically call ensure_table to make sure the required table
   * exists, *unless* $table is unset.
   */
  function add_field($field, $table = 'n', $alias = '') {
    if ($table) {
      if ($table && $table != 'n') {
        $this->ensure_path($table);
        if (!$this->tables[$table]) {
          $this->queue_table($table);
        }
      }
      $table .= ".";
    }
    if ($alias) {
      $a = " AS $alias";
    }
    $this->field[] = "$table$field$a";
  }

  /*
   * Remove all fields that may've been added; primarily used for summary
   * mode where we're changing the query because we didn't get data we needed.
   */
  function clear_fields() {
    $this->field = array();
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
    $this->where .= (!$this->where ? "WHERE " : " AND ") . $clause;
  }

  /*
   * Add an ORDER BY clause to the query. It will ensure the table exists
   * in the query, but it will not ensure the path, so the caller is
   * responsible!
   */
  function add_orderby($table, $field, $order, $alias = '') {
    if (!$alias) {
      $alias = $table;
    }
    if (!$this->tables[$table]) {
      $this->add_table($table, true);
    }

		if ($this->orderby) {
			$this->orderby .= ", ";
    }

		$this->orderby = " $alias.$field $order";
  }

  /*
   * Add multiple an orderby's. Right now I'm not sure why this function
   * is separate from add_orderby above; I don't think it needs to
   * be.
   */
  function add_orderby_multi($table, $field, $order, $alias = '') {
    if (!$alias) {
      $alias = $table;
    }
    if (!$this->tables[$table]) {
      $this->add_table($table, true);
    }

    if (is_array($field)) {
      foreach($field as $f) {
        if ($this->orderby) {
          $this->orderby .= ", ";
        }

        $this->orderby = " $alias.$f $order";
      }
    }
    else {
      if ($this->orderby) {
        $this->orderby .= ", ";
      }

      $this->orderby = " $alias.$field $order";
    }
  }

  /*
   * Add a simple GROUP BY clause to the query. The caller is responsible
   * for ensuring that the fields are fully qualified and the table is properly
   * added.
   */
  function add_groupby($clause) {
    $this->groupby .= (!$this->groupby ? "GROUP BY " : ", ") . $clause;
  }

  /*
   * This function will ensure a table exists; if it already exists it won't
   * do anything, but if it doesn't it will add the table. It will ensure a path
   * leads back to the base table.
   */
  function ensure_table($table) {
    if ($table == 'n' || $table == 'node' || $this->tables[$table]) {
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
   *   If true, it will ensure that a path back to node exists. If false, it
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
    if ($table == 'n' || $table == 'node') {
      return;
    }

    if ($ensure_path && !$this->ensure_path($table)) {
      return false;
    }
    
    if (isset($this->tables[$table])) {
      $this->tables[$table] += $howmany;
    }
    else {
      $this->tables[$table] = $howmany;
    }

    for ($i = $this->tables[$table] - $howmany + 1; $i <= $this->tables[$table]; $i++) {
      if ($joininfo) {
        $this->joins[$table][$i] = $joininfo;
      }
      $this->tablequeue[] = array('table' => $table, 'num' => $i);
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
    $this->tablequeue[] = array('table' => $table, 'num' => $this->tables[$table]);
  }

  /*
   * Set the table header; used for click-sorting because it's needed
   * info to modify the ORDER BY clause.
   */
  function set_header($header) {
    $this->header = $header;
  }

  /*
   * Make sure that the specified table can be properly linked to the base
   * table in the JOINs. This function uses recursion. If the tables
   * needed to complete the path back to the base table are not in the
   * query they will be added, but additional copies will NOT be added
   * if the table is already there.
   */
  function ensure_path($table, $traced = array(), $add = array()) {
    if ($table == 'n' || $table == 'node') {
      return true;
    }
    $table_data = _views_get_tables();

    $left_table = $table_data[$table]['join']['left']['table'];

    // Does it end at 'node'?
    if ($left_table == 'node') {
      // We are done! Add our tables and unwind.
      foreach ($add as $t) {
        // Don't use add_table because it'll trace, and we already did that.
        // And we already know anything in $add isn't already in $this->tables
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
   */
  function get_table_name($table, $table_num) {
    return ($table_num < 2 ? $table : $table . $table_num);
  }

  /*
   * Generate a query and a countquery from all of the information supplied
   * to the object.
   *
   * @param $getcount
   *   Provide a countquery if this is true, otherwise provide a normal query.
   */
  function query($getcount = false) {
//    dprint_r($this);
    $table_data = _views_get_tables();
    // Add all the tables to the query via joins. We assume all LEFT joins.
    foreach ($this->tablequeue as $tinfo) {
      $table = $tinfo['table'];
      $i = $tinfo['num'];
      $tn = $this->get_table_name($table, $i);

      if (!$this->joins[$table][$i]) {
        $ltable = $table_data[$table]['join']['left']['table'];
        $lfield = $table_data[$table]['join']['left']['field'];
        $rfield = $table_data[$table]['join']['right']['field'];
      }
      else {
        $ltable = $this->joins[$table][$i]['ltable'];
        $lfield = $this->joins[$table][$i]['lfield'];
        $rfield = $this->joins[$table][$i]['rfield'];
      }

      // special case for node table
      if ($ltable == 'node') {
        $ltable = 'n'; 
      }
      // Trying to get around problem with \ in PHP 5(?)
      $joins .= ' LEFT JOIN {' . $table. "} $tn ON $ltable.$lfield = $tn.$rfield";
    }

    // If it's not a count query, add our fields
    if (!$getcount) {
      foreach($this->field as $field) {
        $fields .= ($fields ? ", " : "") . $field;
      }
      $groupby = $this->groupby;
    }
    else
      $fields = "count($this->count_field) as numrecs";

    if ($this->header) {
      if ($this->orderby)
        $before = "$this->orderby, ";
      $orderby = tablesort_sql($this->header, $before);
      if (!$orderby && $this->orderby) {
        $orderby = "ORDER BY $this->orderby";
      }
    }
    else if ($this->orderby) {
      $orderby = "ORDER BY $this->orderby";
    }

    $query = "SELECT $fields FROM {node} n $joins $this->where $groupby $orderby";
//    dprint_r($query);
    return $query;
  }
}

/*
 * Add a where clause based on taxonomy. This is a pretty exciting piece of
 * code because the clause and the JOINs are a bit complicated.
 */
function _views_add_taxonomy($op, $value, $depth, &$query) {
  // When filtering via depth, we have to add a chain. If it's an or query
  // we add 1 chain, but in an and query we actually basically have to add
  // a 2 dimensional array. 
  
  $query->ensure_table('term_data'); // this is to make sure the base table comes first.

  if ($op == 'OR') {
    $clause = "'" . implode("','", $value) . "'";
    // base tables

    // Any number of things might have already added a term_node table, but
    // if they ahve and are filtering on it the query is going to be borken
    // anyway. If it's been added and unfiltered, and we ignore it, we get
    // bad results.
    $num = 1;
    $query->ensure_table('term_node');
    $tablename = $query->get_table_name('term_node', $num);
    $where = "$tablename.tid IN ($clause)";

    // for each depth > 0, add the next child in term_hierarchy to the join
    for ($i = 0; $i < $depth; $i++) {
      $thnum = $query->add_table('term_hierarchy', false, 1, array('ltable' => $tablename, 'lfield' => 'tid', 'rfield' => 'tid'));
      $thname = $query->get_table_name('term_hierarchy', $thnum);
      $num = $query->add_table('term_data', false, 1, array('ltable' => $thname, 'lfield' => 'parent', 'rfield' => 'tid'));
      $tablename = $query->get_table_name('term_data', $num);
      $where .= " OR $tablename.tid IN ($clause)";
    }
    $query->add_where("($where)");
  }
  else {
    if ($op == 'NOR') {
      $not = "!";
    }
    foreach ($value as $tid) {
      // For every term we have to match, add a base table
      $num = 1;
      $query->ensure_table('term_node');
      $tablename = $query->get_table_name('term_node', $num);
      $where = "$tablename.tid = '$tid'";

      // for each depth > 0, add the next child in term_hierarchy to the join
      for ($i = 0; $i < $depth; $i++) {
        $thnum = $query->add_table('term_hierarchy', false, 1, array('ltable' => $tablename, 'lfield' => 'tid', 'rfield' => 'tid'));
        $thname = $query->get_table_name('term_hierarchy', $thnum);
        $num = $query->add_table('term_data', false, 1, array('ltable' => $thname, 'lfield' => 'parent', 'rfield' => 'tid'));
        $tablename = $query->get_table_name('term_data', $num);
        $where .= " OR $tablename.tid = '$tid'";
      }
      $query->add_where("$not($where)");
    }
  }
}

/*
 * Break x,y,z and x+y+z into an array.
 */
function _views_break_phrase($str) {
  if (preg_match('/^([0-9]+[+ ])+[0-9]+$/', $str)) {
    // The '+' character in a query string may be parsed as ' '.
    return array('or', preg_split('/[+ ]/', $str));
  }
  else if (preg_match('/^([0-9]+,)*[0-9]+$/', $str)) {
    return array('and', explode(',', $str));
  }
  else {
    return NULL;
  }
}

