<?php
// $Id$
/**
 * @file query.inc
 * Defines the query object which is the underlying layer in a View.
 */

/**
 * Object used to create a SELECT query.
 */
class views_query {

  /**
   * A list of tables in the order they should be added, keyed by alias.
   */
  var $table_queue = array();

  /**
   * Holds an array of tables and counts added so that we can create aliases
   */
  var $tables = array();

  /**
   * Holds an array of relationships, which are aliases of the primary
   * table that represent different ways to join the same table in.
   */
  var $relationships = array();

  /**
   * An array of sections of the WHERE query. Each section is in itself
   * an array of pieces and a flag as to whether or not it should be AND
   * or OR.
   */
  var $where = array();
  /**
   * An array of sections of the HAVING query. Each section is in itself
   * an array of pieces and a flag as to whether or not it should be AND
   * or OR.
   */
  var $having = array();
  /**
   * The default operator to use when connecting the WHERE groups. May be
   * AND or OR.
   */
  var $group_operator = 'AND';

  /**
   * A simple array of order by clauses.
   */
  var $orderby = array();

  /**
   * A simple array of group by clauses.
   */
  var $groupby = array();

  /**
   * The table header to use for tablesort. This matters because tablesort
   * needs to modify the query and needs the header.
   */
  var $header = array();

  /**
   * A flag as to whether or not to make the primary field distinct.
   */
  var $distinct = FALSE;

  /**
   * Constructor; Create the basic query object and fill with default values.
   */
  function views_query($primary_table = 'node', $primary_field = 'nid') {
    $this->primary_table = $primary_table;  // Predefine these above, for clarity.
    $this->primary_field = $primary_field;
    $this->relationships[$primary_table] = array(
      'link' => NULL,
      'table' => $primary_table,
      'alias' => $primary_table,
      'base' => $primary_table
    );

    // init the table queue with our primary table.
    $this->table_queue[$primary_table] = array(
      'alias' => $primary_table,
      'table' => $primary_table,
      'relationship' => $primary_table,
      'join' => NULL,
    );

    // init the tables with our primary table
    $this->tables[$primary_table][$primary_table] = array(
      'count' => 1,
      'alias' => $primary_table,
    );

    if ($primary_field) {
      $this->fields[$primary_field] = array(
        'table' => $primary_table,
        'field' => $primary_field,
        'alias' => $primary_field,
      );
    }

    $this->count_field = array(
      'table' => $primary_table,
      'field' => $primary_field,
      'alias' => $primary_field,
      'count' => TRUE,
    );
  }

  // ----------------------------------------------------------------
  // Utility methods to set flags and data.

  /**
   * Set the base field to be distinct.
   */
  function set_distinct($value = TRUE) {
    if (!(isset($this->no_distinct) && $value)) {
      $this->distinct = $value;
    }
  }

  /**
   * Set what field the query will count() on for paging.
   */
  function set_count_field($table, $field, $alias = NULL) {
    if (empty($alias)) {
      $alias = $table . '_' . $field;
    }
    $this->count_field = array(
      'table' => $table,
      'field' => $field,
      'alias' => $alias,
      'count' => TRUE,
    );
  }

  /**
   * Set the table header; used for click-sorting because it's needed
   * info to modify the ORDER BY clause.
   */
  function set_header($header) {
    $this->header = $header;
  }

  // ----------------------------------------------------------------
  // Table/join adding

  /**
   * A relationship is an alternative endpoint to a series of table
   * joins. Relationships must be aliases of the primary table and
   * they must join either to the primary table or to a pre-existing
   * relationship.
   *
   * An example of a relationship would be a nodereference table.
   * If you have a nodereference named 'book_parent' which links to a
   * parent node, you could set up a relationship 'node_book_parent'
   * to 'node'. Then, anything that links to 'node' can link to
   * 'node_book_parent' instead, thus allowing all properties of
   * both nodes to be available in the query.
   *
   * @param $alias
   *   What this relationship will be called, and is also the alias
   *   for the table.
   * @param $join
   *   A views_join object (or derived object) to join the alias in.
   * @param $base
   *   The name of the 'base' table this relationship represents; this
   *   tells the join search which path to attempt to use when finding
   *   the path to this relationship.
   * @param $link_point
   *   If this relationship links to something other than the primary
   *   table, specify that table here. For example, a 'track' node
   *   might have a relationship to an 'album' node, which might
   *   have a relationship to an 'artist' node.
   */
  function add_relationship($alias, $join, $base, $link_point = NULL) {
    if (empty($link_point)) {
      $link_point = $this->primary_table;
    }
    else if (!array_key_exists($link_point, $this->relationships)) {
      return FALSE;
    }

    $join = $this->adjust_join($join, $link_point);
    if ($alias = $this->add_table($join->table, $link_point, $join, $alias)) {
      $this->relationships[$alias] = array(
        'link' => $link_point,
        'table' => $join->table,
        'base' => $base,
      );
      return $alias;
    }
  }

  /**
   * Add a table to the query, ensuring the path exists.
   *
   * This function will test to ensure that the path back to the primary
   * table is valid and exists; if you do not wish for this testing to
   * occur, use $query->queue_table() instead.
   *
   * @param $table
   *   The name of the table to add. It needs to exist in the global table
   *   array.
   * @param $relationship
   *   An alias of a table; if this is set, the path back to this table will
   *   be tested prior to adding the table, making sure that all intermediary
   *   tables exist and are properly aliased. If set to NULL the path to
   *   the primary table will be ensured. If the path cannot be made, the
   *   table will NOT be added.
   * @param $join
   *   In some join configurations this table may actually join back through
   *   a different method; this is most likely to be used when tracing
   *   a hierarchy path. (node->parent->parent2->parent3). This parameter
   *   will specify how this table joins if it is not the default.
   * @param $alias
   *   A specific alias to use, rather than the default alias.
   *
   * @return $alias
   *   The alias of the table; this alias can be used to access information
   *   about the table and should always be used to refer to the table when
   *   adding parts to the query. Or FALSE if the table was not able to be
   *   added.
   */
  function add_table($table, $relationship = NULL, $join = NULL, $alias = NULL) {
    if (!$this->ensure_path($table, $relationship, $join)) {
      return FALSE;
    }

    return $this->queue_table($table, $relationship, $join, $alias);
  }

  /**
   * Add a table to the query, without ensuring the path.
   *
   * This function will test to ensure that the path back to the primary
   * table is valid and exists; if you do not wish for this testing to
   * occur, use $query->queue_table() instead.
   *
   * @param $table
   *   The name of the table to add. It needs to exist in the global table
   *   array.
   * @param $relationship
   *   The primary table alias this table is related to. If not set, the
   *   primary table will be used.
   * @param $join
   *   In some join configurations this table may actually join back through
   *   a different method; this is most likely to be used when tracing
   *   a hierarchy path. (node->parent->parent2->parent3). This parameter
   *   will specify how this table joins if it is not the default.
   * @param $alias
   *   A specific alias to use, rather than the default alias.
   *
   * @return $alias
   *   The alias of the table; this alias can be used to access information
   *   about the table and should always be used to refer to the table when
   *   adding parts to the query. Or FALSE if the table was not able to be
   *   added.
   */
  function queue_table($table, $relationship = NULL, $join = NULL, $alias = NULL) {
    // If the alias is set, make sure it doesn't already exist.
    if (isset($this->table_queue[$alias])) {
      return TRUE;
    }

    if (empty($relationship)) {
      $relationship = $this->primary_table;
    }

    if (!array_key_exists($relationship, $this->relationships)) {
      return FALSE;
    }

    $alias = $this->mark_table($table, $relationship, $alias);

    // If no alias is specified, give it the default.
    if (!isset($alias)) {
      $alias = $this->tables[$relationship][$table]['alias'] . $this->tables[$relationship][$table]['count'];
    }

    // If this is a relationship based table, add a marker with
    // the relationship as a primary table for the alias.
    if ($table!= $alias) {
      $this->mark_table($alias, $this->primary_table, $alias);
    }

    // If no join is specified, pull it from the table data.
    if (!isset($join)) {
      $join = $this->get_join_data($table, $this->relationships[$relationship]['base']);
      if (empty($join)) {
        return FALSE;
      }

      $join = $this->adjust_join($join, $relationship);
    }

    $this->table_queue[$alias] = array(
      'table' => $table,
      'num' => $this->tables[$relationship][$table]['count'],
      'alias' => $alias,
      'join' => $join,
      'relationship' => $relationship,
    );

    return $alias;
  }

  function mark_table($table, $relationship, $alias) {
    // Mark that this table has been added.
    if (empty($this->tables[$relationship][$table])) {
      if (!isset($alias)) {
        $alias = '';
        if ($relationship != $this->primary_table) {
          // double underscore will help prevent accidental name
          // space collisions.
          $alias = $relationship . '__';
        }
        $alias .= $table;
      }
      $this->tables[$relationship][$table] = array(
        'count' => 1,
        'alias' => $alias,
      );
    }
    else {
      $this->tables[$relationship][$table]['count']++;
    }

    return $alias;
  }

  /**
   * Ensure a table exists in the queue; if it already exists it won't
   * do anything, but if it doesn't it will add the table queue. It will ensure
   * a path leads back to the relationship table.
   *
   * @param $table
   *   The unaliased name of the table to ensure.
   * @param $relationship
   *   The relationship to ensure the table links to. Each relationship will
   *   get a unique instance of the table being added. If not specified,
   *   will be the primary table.
   * @param $join
   *   A views_join object (or derived object) to join the alias in.
   *
   * @return
   *   The alias used to refer to this specific table, or NULL if the table
   *   cannot be ensured.
   */
  function ensure_table($table, $relationship = NULL, $join = NULL) {
    // ensure a relationship
    if (empty($relationship)) {
      $relationship = $this->primary_table;
    }

    // If the relationship is the primary table, this actually be a relationship
    // link back from an alias. We store all aliases along with the primary table
    // to detect this state, because eventually it'll hit a table we already
    // have and that's when we want to stop.
    if ($relationship == $this->primary_table && !empty($this->tables[$relationship][$table])) {
      return $this->tables[$relationship][$table]['alias'];
    }

    if (!array_key_exists($relationship, $this->relationships)) {
      return FALSE;
    }

    if ($table == $this->relationships[$relationship]['base']) {
      return $relationship;
    }

    if (!empty($join)) {
      $join = $this->adjust_join($join, $relationship);
    }

    if ($this->ensure_path($table, $relationship, $join)) {
      return $this->queue_table($table, $relationship, $join);
    }
  }

  /**
   * Make sure that the specified table can be properly linked to the primary
   * table in the JOINs. This function uses recursion. If the tables
   * needed to complete the path back to the primary table are not in the
   * query they will be added, but additional copies will NOT be added
   * if the table is already there.
   */
  function ensure_path($table, $relationship = NULL, $join = NULL, $traced = array(), $add = array()) {
    if (!isset($relationship)) {
      $relationship = $this->primary_table;
    }

    if (!array_key_exists($relationship, $this->relationships)) {
      return FALSE;
    }

    if (!isset($join)) {
      $join = $this->get_join_data($table, $this->relationships[$relationship]['base']);
    }

    // Does a table along this path exist?
    if (isset($this->tables[$relationship][$table]) ||
      ($join && $join->left_table == $relationship) ||
      ($join && $join->left_table == $this->relationships[$relationship]['table'])) {
      // Make sure that we're linking to the correct table for our relationship.
      foreach (array_reverse($add) as $table => $join) {
        $this->queue_table($table, $relationship, $this->adjust_join($join, $relationship));
      }
      return TRUE;
    }

    // Have we been this way?
    if (isset($traced[$join->left_table])) {
      // we looped. Broked.
      return FALSE;
    }

    // Do we have to add this table?
    $left_join = $this->get_join_data($join->left_table, $this->relationships[$relationship]['base']);
    if (!isset($this->tables[$relationship][$join->left_table])) {
      $add[$join->left_table] = $left_join;
    }

    // Keep looking.
    $traced[$join->left_table] = TRUE;
    return $this->ensure_path($join->left_table, $relationship, $left_join, $traced, $add);
  }

  /**
   * Fix a join to adhere to the proper relationship; the left table can vary
   * based upon what relationship items are joined in on.
   */
  function adjust_join($join, $relationship) {
    // Adjusts the left table for our relationship.
    if ($relationship != $this->primary_table) {
      // If we're linking to the primary table, the relationship to use will
      // be the prior relationship. Unless it's a direct link.

      // Safety! Don't modify an original here.
      $join = drupal_clone($join);

      // First, if this is our link point/anchor table, just use the relationship
      if ($join->left_table == $this->relationships[$relationship]['table']) {
        $join->left_table = $relationship;
      }
      // then, try the base alias.
      else if (isset($this->tables[$relationship][$join->left_table]['alias'])) {
        $join->left_table = $this->tables[$relationship][$join->left_table]['alias'];
      }
      // But if we're already looking at an alias, use that instead.
      else if (isset($this->table_queue[$relationship]['alias'])) {
        $join->left_table = $this->table_queue[$relationship]['alias'];
      }
    }
    return $join;
  }

  /**
   * Retrieve join data from the larger join data cache.
   *
   * @param
   *   The table to get the join information for.
   * @param
   *   The path we're following to get this join.
   *
   * @return
   *   A views_join object or child object, if one exists.
   */
  function get_join_data($table, $primary_table) {
    // Check to see if we're linking to a known alias. If so, get the real
    // table's data instead.
    if (!empty($this->table_queue[$table])) {
      $table = $this->table_queue[$table]['table'];
    }
    return views_get_table_join($table, $primary_table);

  }

  /**
   * Get the information associated with a table.
   *
   * If you need the alias of a table with a particular relationship, use
   * ensure_table().
   */
  function get_table_info($table) {
    if (!empty($this->table_queue[$table])) {
      return $this->table_queue[$table];
    }

    // In rare cases we might *only* have aliased versions of the table.
    if (!empty($this->tables[$this->primary_table][$table])) {
      $alias = $this->tables[$this->primary_table][$table]['alias'];
      if (!empty($this->table_queue[$alias])) {
        return $this->table_queue[$alias];
      }
    }
  }

  /**
   * Add a field to the query table, possibly with an alias. This will
   * automatically call ensure_table to make sure the required table
   * exists, *unless* $table is unset.
   *
   * @param $table
   *   The table this field is attached to. If NULL, it is assumed this will
   *   be a formula; otherwise, ensure_table is used to make sure the
   *   table exists.
   * @param $field
   *   The name of the field to add. This may be a real field or a formula.
   * @param $alias
   *   The alias to create. If not specified, the alias will be $table_$field
   *   unless $table is NULL. When adding formulae, it is recommended that an
   *   alias be used.
   *
   * @return $name
   *   The name that this field can be referred to as. Usually this is the alias.
   */
  function add_field($table, $field, $alias = '') {
    // We check for this specifically because it gets a special alias.
    if ($table == $this->primary_table && $field == $this->primary_field && empty($alias)) {
      $alias = $this->primary_field;
    }

    if ($table) {
      $this->ensure_table($table);
    }

    if (!$alias && $table) {
      $alias = $table . '_' . $field;
    }

    $name = $alias ? $alias : $field;

    if (empty($this->fields[$alias])) {
      $this->fields[$name] = array(
        'field' => $field,
        'table' => $table,
        'alias' => $alias,
      );
    }
    return $name;
  }

  /**
   * Remove all fields that may've been added; primarily used for summary
   * mode where we're changing the query because we didn't get data we needed.
   */
  function clear_fields() {
    $this->fields = array();
  }

  /**
   * Create a new grouping for the WHERE or HAVING clause.
   *
   * @param $type
   *   Either 'AND' or 'OR'. All items within this group will be added
   *   to the WHERE clause with this logical operator.
   * @param $group
   *   An ID to use for this group. If unspecified, an ID will be generated.
   * @param $where
   *   'where' or 'having'.
   *
   * @return $group
   *   The group ID generated.
   */
  function set_where_group($type = 'AND', $group = NULL, $where = 'where') {
    // Set an alias.
    $groups = &$this->$where;

    if (!isset($group)) {
      $group = max(array_keys($groups)) + 1;
    }

    // Create an empty group
    if (empty($groups[$group])) {
      $groups[$group] = array('clauses' => array(), 'args' => array());
    }

    $groups[$group]['type'] = strtoupper($type);
    return $group;
  }

  /**
   * Control how all WHERE and HAVING groups are put together.
   *
   * @param $type
   *   Either 'AND' or 'OR'
   */
  function set_group_operator($type = 'AND') {
    $this->group_operator = strtoupper($type);
  }

  /**
   * Add a simple WHERE clause to the query. The caller is responsible for
   * ensuring that all fields are fully qualified (TABLE.FIELD) and that
   * the table already exists in the query.
   *
   * @param $group
   *   The WHERE group to add these to; groups are used to create AND/OR
   *   sections. Groups cannot be nested. Use 0 as the default group.
   *   If the group does not yet exist it will be created as an AND group.
   * @param $clause
   *   The actual clause to add. When adding a where clause it is important
   *   that all tables are addressed by the alias provided by add_table or
   *   ensure_table and that all fields are addressed by their alias wehn
   *   possible. Please use %d and %s for arguments.
   * @param ...
   *   A number of arguments as used in db_query(). May be many args or one
   *   array full of args.
   */
  function add_where($group, $clause) {
    $args = func_get_args();
    array_shift($args); // ditch $group
    array_shift($args); // ditch $clause

    // Expand an array of args if it came in.
    if (count($args) == 1 && is_array(reset($args))) {
      $args = current($args);
    }

    // Ensure all variants of 0 are actually 0. Thus '', 0 and NULL are all
    // the default group.
    if (empty($group)) {
      $group = 0;
    }

    // Check for a group.
    if (!isset($this->where[$group])) {
      $this->set_where_group('AND', $group);
    }

    // Add the clause and the args.
    if (is_array($args)) {
      $this->where[$group]['clauses'][] = $clause;
      $this->where[$group]['args'] = array_merge($this->where[$group]['args'], $args);
    }
  }

  /**
   * Add a simple HAVING clause to the query. The caller is responsible for
   * ensuring that all fields are fully qualified (TABLE.FIELD) and that
   * the table and an appropriate GROUP BY already exist in the query.
   *
   * @param $group
   *   The HAVING group to add these to; groups are used to create AND/OR
   *   sections. Groups cannot be nested. Use 0 as the default group.
   *   If the group does not yet exist it will be created as an AND group.
   * @param $clause
   *   The actual clause to add. When adding a having clause it is important
   *   that all tables are addressed by the alias provided by add_table or
   *   ensure_table and that all fields are addressed by their alias wehn
   *   possible. Please use %d and %s for arguments.
   * @param ...
   *   A number of arguments as used in db_query(). May be many args or one
   *   array full of args.
   */
  function add_having($group, $clause) {
    $args = func_get_args();
    array_shift($args); // ditch $group
    array_shift($args); // ditch $clause

    // Expand an array of args if it came in.
    if (count($args) == 1 && is_array(reset($args))) {
      $args = current($args);
    }

    // Ensure all variants of 0 are actually 0. Thus '', 0 and NULL are all
    // the default group.
    if (empty($group)) {
      $group = 0;
    }

    // Check for a group.
    if (!isset($this->having[$group])) {
      $this->set_where_group('AND', $group, 'having');
    }

    // Add the clause and the args.
    if (is_array($args)) {
      $this->having[$group]['clauses'][] = $clause;
      $this->having[$group]['args'] = array_merge($this->having[$group]['args'], $args);
    }
  }

  /**
   * Add multiple an orderby's. Right now I'm not sure why this function
   * is separate from add_orderby above; I don't think it needs to
   * be.
   *
   * @param $table
   *   The table this field is part of. If a formula, enter NULL.
   * @param $field
   *   The field or formula to sort on. If already a field, enter NULL
   *   and put in the alias.
   * @param $order
   *   Either ASC or DESC.
   * @param $alias
   *   The alias to add the field as. In SQL, all fields in the order by
   *   must also be in the SELECT portion. If an $alias isn't specified
   *   one will be generated for from the $field; however, if the
   *   $field is a formula, this alias will likely fail.
   *
   * @return $alias
   *   The alias created.
   */
  function add_orderby($table, $field, $order, $alias = '') {
    if ($table) {
      $this->ensure_table($table);
    }

    // Only fill out this aliasing if there is a table;
    // otherwise we assume it is a formula.
    if (!$alias && $table) {
      $as = $table . '_' . $field;
    }
    else {
      $as = $alias;
    }

    if ($field) {
      $this->add_field($table, $field, $as);
    }

    $this->orderby[] = "$as $order";

    // If grouping, all items in the order by must also be in the
    // group by clause.
    if ($this->groupby) {
      $this->add_groupby($as);
    }
  }

  /**
   * Add a simple GROUP BY clause to the query. The caller is responsible
   * for ensuring that the fields are fully qualified and the table is properly
   * added.
   */
  function add_groupby($clause) {
    // Only add it if it's already in there.
    if (!in_array($clause, $this->groupby)) {
      $this->groupby[] = $clause;
    }
  }
  
  /**
   * Construct the "WHERE" or "HAVING" part of the query.
   *
   * @param $where
   *   'where' or 'having'.
   */
  function condition_sql($where = 'where') {
    $clauses = array();
    foreach ($this->$where as $group => $info) {
      $clause = implode(") " . $info['type'] . " (", $info['clauses']);
      if (count($info['clauses']) > 1) {
        $clause = '(' . $clause . ')';
      }
      $clauses[] = $clause;
    }

    if ($clauses) {
      $keyword = drupal_strtoupper($where);
      if (count($clauses) > 1) {
        return "$keyword (" . implode(")\n    " . $this->group_operator . ' (', $clauses) . ")\n";
      }
      else {
        return "$keyword " . array_shift($clauses) . "\n";
      }
    }
    return "";
  }

  /**
   * Generate a query and a countquery from all of the information supplied
   * to the object.
   *
   * @param $get_count
   *   Provide a countquery if this is true, otherwise provide a normal query.
   */
  function query($get_count = FALSE) {
    // Check query distinct value.
    if (empty($this->no_distinct) && $this->distinct && !empty($this->fields)) {
      $this->fields[$this->primary_field]['distinct'] = TRUE;
      $this->count_field['distinct'] = TRUE;
    }

    $joins = $fields = $where = $having = $orderby = $groupby = '';
    // Add all the tables to the query via joins. We assume all LEFT joins.
    foreach ($this->table_queue as $table) {
      if (is_object($table['join'])) {
        $joins .= $table['join']->join($table, $this) . "\n";
      }
    }

    // If it's not a count query, add our fields
    $fields_array = $this->fields;
    foreach ($fields_array as $field) {
      if ($fields) {
        $fields .= ",\n   ";
      }
      $string = '';
      if ($field['table']) {
        $string .= $field['table'] . '.';
      }
      $string .= $field['field'];
      if (isset($field['distinct'])) {
        $string = "DISTINCT($string)";
      }
      if (isset($field['count'])) {
        $string = "COUNT($string)";
      }
      if ($field['alias']) {
        $string .= " AS $field[alias]";
      }
      $fields .= $string;
    }

    if ($this->groupby) {
      $groupby = "GROUP BY " . implode(', ', $this->groupby) . "\n";
      if ($this->having) {
        $having = $this->condition_sql('having');
      }
    }

    if (!$get_count) {
      // we only add the groupby if we're not counting.
      if ($this->orderby) {
        $orderby = "ORDER BY " . implode(', ', $this->orderby) . "\n";
      }
    }

    $where = $this->condition_sql();

    $query = "SELECT $fields\n FROM {" . $this->primary_table . "} $this->primary_table \n$joins $where $groupby $having $orderby";

    if ($get_count) {
      $query = "SELECT COUNT(*) FROM ($query) count_alias";
    }

    $replace = array('&gt;' => '>', '&lt;' => '<');
    $query = strtr($query, $replace);

    return $query;
  }

  /**
   * Get the arguments attached to the WHERE and HAVING clauses of this query.
   */
  function get_where_args() {
    $args = array();
    foreach ($this->where as $group => $where) {
      $args = array_merge($args, $where['args']);
    }
    foreach ($this->having as $group => $having) {
      $args = array_merge($args, $having['args']);
    }
    return $args;
  }
}

