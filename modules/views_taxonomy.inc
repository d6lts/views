<?php
// $Id$

/**
 * This include file implements views functionality on behalf of taxonomy.module
 */

function taxonomy_views_tables() {
  if (module_exist('taxonomy')) {
    $table = views_new_table('term_node', 'internal', 'node', 'nid', 'nid');
    views_table_add_field($table, 'name', 'Taxonomy: All Terms', 'This will display all taxonomy terms associated with the node. Note that this causes one extra query per row displayed, and might have a minor performance impact.', array(
      'sortable' => false,
      'handler' => 'views_handler_field_allterms',
      'notafield' => true,
    ));

    views_table_add_filter($table, 'tid', 'Taxonomy: Term', 'When filtering by taxonomy term you may specify the "depth" as an option. Please see the taxonomy help for more information.', array(
      'list' => 'views_handler_filter_tid',
      'option' => 'string',
      'operator' => 'views_handler_operator_andor',
      'handler' => 'views_handler_filter_tid_custom',
      'value-type' => 'array',
    ));

    $tables[$table['name']] = $table;

    $vocabularies = taxonomy_get_vocabularies();
    foreach ($vocabularies as $voc) {
      $tables["term_node_$voc->vid"] = array(
        'name' => 'term_node',
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
          'name' => array(
            'name' => t('Taxonomy: Terms for %voc-name', array('%voc-name' => $voc->name)),
            'sortable' => false,
            'help' => t('This will display all taxonomy terms associated with the node that are members of %voc-name. Note that this causes one extra query per row displayed, and might have a minor performance impact.', array('%voc-name' => $voc->name)),
            'handler' => 'views_handler_field_allterms',
            'vocabulary' => $voc->vid,
            'notafield' => true,
          ),
        ),
        'filters' => array(
          'tid' => array(
            'name' => t('Taxonomy: Terms for %voc-name', array('%voc-name' => $voc->name)),
            'list' => 'views_handler_filter_tid_by_voc',
            'value-type' => 'array',
            'option' => 'string',
            'operator' => 'views_handler_operator_andor',
            'handler' => 'views_handler_filter_tid_custom',
            'vocabulary' => $voc->vid,
            'help' => t("Only terms associated with %voc-name will appear in the select box for this filter. When filtering by taxonomy term you may specify the 'depth' as an option. Please see the taxonomy help for more information.", array('%voc-name' => $voc->name)),
          ),
        )
      );

    }

    $tables['term_hierarchy'] = array(
      'name' => 'term_hierarchy',
      'provider' => 'internal',
      'join' => array(
        'left' => array(
          'table' => 'term_node',
          'field' => 'tid'
        ),
        'right' => array(
          'field' => 'tid'
        )
      ),
    );

    $tables['term_data'] = array(
      'name' => 'term_data',
      'provider' => 'internal',
      'join' => array(
        'left' => array(
          'table' => 'term_node',
          'field' => 'tid'
        ),
        'right' => array(
          'field' => 'tid'
        ),
      ),
      'fields' => array(
        'name' => array(
          'name' => t('Taxonomy: Term'),
          'sortable' => true,
          'handler' => array('views_handler_field_tid' => "No link", 'views_handler_field_tid_link' => "With link"),
          'addlfields' => array('tid'),
          'help' => t('This will display one of the taxonomy terms associated with the node; if taxonomy terms were used to filter or sort, it will be the one that triggered the sort or filter.'),
        ),
        'description' => array(
          'name' => t('Taxonomy: Term Description'),
          'sortable' => false,
          'help' => t('This will display the description associated with a taxonomy term.'),
        ),
      ),
      'sorts' => array(
        'weight' => array(
          'name' => t('Taxonomy: Term Name'),
          'field' => array('weight', 'name'),
          'help' => t('This will sort nodes by taxonomy weight and name, as defined in the category administration.'),
        )
      ),
      'filters' => array(
        'vid' => array(
          'name' => t('Taxonomy: Vocabulary Name'),
          'list' => 'views_handler_filter_vid',
          'operator' => 'views_handler_operator_andor',
          'handler' => 'views_handler_filter_voc',
          'value-type' => 'array',
          'help' => t('This will filter a view to only nodes that contain a term in the associated vocabulary.'),
        )
      )
    );
    $table = views_new_table('vocabulary', 'internal', 'term_data', 'vid', 'vid');
    $tables['vocabulary'] = $table;

  }
  return $tables;
}

function taxonomy_views_arguments() {
  $arguments = array(
    'taxid' => array(
      'name' => t('Taxonomy: Term ID'),
      'handler' => 'views_handler_arg_taxid',
      'option' => 'string',
      'help' => t('The argument will filter by a taxonomy term ID. For this argument, set the option to the depth to search. See taxonomy for more information.'),
    ),
    'taxletter' => array(
      'name' => t('Taxonomy: Term Name'),
      'handler' => 'views_handler_arg_taxletter',
      'option' => 'string',
      'help' => t('The argument will filter by a taxonomy term name. For this argument, set the option to the number of characters, using 0 for full term; use 1 for an A/B/C style glossary.'),
    ),
    'vocid' => array(
      'name' => t('Taxonomy: Vocabulary ID'),
      'handler' => 'views_handler_arg_vocid',
      'help' => t('The argument will filter to nodes with terms in a vocabulary.'),
    ),
  );
  return $arguments;
}

function taxonomy_views_default_views() {
  $view = new stdClass();
  $view->name = 'taxonomy_term';
  $view->disabled = true;
  $view->description = t('The taxonomy view with a depth of 0.');
  $view->access = array (
);
  $view->page = TRUE;
  $view->page_title = t('taxonomy');
  $view->page_header = '';
  $view->page_header_format = '1';
  $view->page_type = 'teaser';
  $view->url = 'taxonomy/term';
  $view->use_pager = TRUE;
  $view->nodes_per_page = '10';
  $view->sort = array (
    array (
      'tablename' => 'node',
      'field' => 'sticky',
      'sortorder' => 'DESC',
      'options' => '',
    ),
    array (
      'tablename' => 'node',
      'field' => 'created',
      'sortorder' => 'DESC',
      'options' => '',
    ),
  );
  $view->argument = array (
    array (
      'type' => 'taxid',
      'argdefault' => '1',
      'title' => '%1',
      'options' => '0',
    ),
    array (
      'type' => 'node_feed',
      'argdefault' => '2',
      'title' => '',
      'options' => '',
    ),
  );
  $view->field = array (
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
  $view->requires = array(node);
  $views[$view->name] = $view;
  return $views;
}

/**
 * invalidate the views cache when taxonomy vocabulary changes.
 */
function views_taxonomy($op, $type, $object = NULL) {
  if ($type == 'vocabulary' && $op == 'delete' || $op == 'insert' || $op == 'update') {
    views_invalidate_cache();
  }
}

/**
 * Display all the terms for a given vocabulary
 */
function views_handler_field_allterms($fieldinfo, $fielddata, $value, $data) {
  if ($fieldinfo['vocabulary']) {
    $terms = taxonomy_node_get_terms_by_vocabulary($data->nid, $fieldinfo['vocabulary']);
  }
  else {
    $terms = taxonomy_node_get_terms($data->nid);
  }

  $node->taxonomy = $terms;
  return theme('links', taxonomy_link('taxonomy terms', $node));
}

/**
 * Display a simple taxonomy term
 */
function views_handler_field_tid($fieldinfo, $fielddata, $value, $data) {
  return check_plain($value);
}

/**
 * Display a link to a taxonomy term
 */
function views_handler_field_tid_link($fieldinfo, $fielddata, $value, $data) {
  $fieldname = $fielddata['tablename'] . '_tid';
  $tid = $data->$fieldname;
  return l($value, "taxonomy/term/$tid");
}


function views_handler_arg_taxid($op, &$query, $argtype, $arg = '') {
  switch($op) {
    case 'summary':
      $query->ensure_table('term_data', true);
      $query->add_field('name', 'term_data');
      $query->add_field('weight', 'term_data');
      $query->add_field('tid', 'term_data');
      $fieldinfo['field'] = "term_data.name";
      return $fieldinfo;
    case 'sort':
      $query->add_orderby('term_data', 'weight', $argtype);
      $query->add_orderby('term_data', 'name', $argtype);
      break;
    case 'filter':
      if ($arg == 0) { // untagged only!
        $query->ensure_table("term_node");
        $query->add_where("term_node.tid IS NULL");
      }
      else {
        $values = _views_break_phrase($arg);
        _views_add_taxonomy(strtoupper($values[0]), $values[1], $argtype['options'], $query);
      }
      break;
    case 'link':
      $name = ($query->name ? $query->name : t('Uncategorized'));
      return l($name, "$arg/" . intval($query->tid));
    case 'title':
      if (!$query) {
        return t('Uncategorized');
      }
      list($type, $info) = _views_break_phrase($query);
      if (!$info) {
        return t('Uncategorized');
      }
      $tids = implode(',', $info); // only does numbers so safe

      $result = db_query("SELECT name FROM {term_data} WHERE tid IN (%s)", $tids);
      while ($term = db_fetch_object($result)) {
        $title .= ($title ? ($type == 'or' ? ' + ' : ', ') : '') . check_plain($term->name);
      }
      return $title;
  }
}

function views_handler_arg_vocid($op, &$query, $argtype, $arg = '') {
  switch($op) {
    case 'summary':
      $query->ensure_table('vocabulary');
      $query->add_field('name', 'vocabulary');
      $query->add_field('vid', 'vocabulary');
      $fieldinfo['field'] = "vocabulary.name";
      return $fieldinfo;
    case 'sort':
      $query->add_orderby('vocabulary', 'weight', $argtype);
      $query->add_orderby('vocabulary', 'name', $argtype);
      break;
    case 'filter':
      $query->ensure_table('vocabulary');
      $query->add_where('vocabulary.vid = %d', $arg);
      $query->set_distinct();
      break;
    case 'link':
      return l($query->name, "$arg/" . intval($query->vid));
    case 'title':
      $result = db_query("SELECT name FROM {vocabulary} WHERE vid = %d", $query);
      $voc = db_fetch_object($result);
      $title = check_plain($voc->name);
      return $title;
  }
}

function views_handler_arg_taxletter($op, &$query, $argtype, $arg = '') {
  switch($op) {
    case 'summary':
      $query->add_table('term_data', true);
      $len = intval($arg);
      $fieldinfo['field'] = ($len <= 0 ? "term_data.name" : "LEFT(term_data.name, $len)");

      $fieldinfo['fieldname'] = 'letter';
      $query->add_field('tid', 'term_data');
      $query->add_where('term_data.name IS NOT NULL');
      return $fieldinfo;
      break;
    case 'sort':
      $query->add_orderby('term_data', 'name', $argtype);
      break;
    case 'filter':
      $len = intval($argtype['options']);
      $query->add_table('term_data', true);

      if ($len <= 0) {
        $query->add_where("term_data.name = '%s'", $arg);
      } else {
        $query->add_where("LEFT(term_data.name, $len) = '%s'", $arg);
      }
      break;
    case 'link':
      return l($query->letter, "$arg/$query->letter");
    case 'title':
      return check_plain($query);
  }
}

/*
 * Create a simple list of all terms.
 */
function views_handler_filter_tid($op, $filterinfo) {
  $tids = array();
  if ($filterinfo['vocabulary']) {
    $where = "WHERE td.vid = $filterinfo[vocabulary]";
  }
  $result = db_query("SELECT DISTINCT(td.tid), td.name, v.name as vocabname FROM {term_data} td LEFT JOIN {vocabulary} v ON v.vid = td.vid $where ORDER BY v.weight, v.name, td.weight, td.name");
  while ($obj = db_fetch_object($result)) {
    if ($filterinfo['vocabulary']) {
      $tids[$obj->tid] = "$obj->name";
    }
    else {
      $tids[$obj->tid] = "$obj->vocabname: $obj->name";
    }
  }

  return $tids;
}

/**
 * Create a tree list of terms for a single vocabulary
 */
function views_handler_filter_tid_by_voc($op, $filterinfo) {
  $tree = taxonomy_get_tree($filterinfo[vocabulary]);
  $options = array();

  if ($tree) {
    foreach ($tree as $term) {
      $options[$term->tid] = _taxonomy_depth($term->depth, '-') . $term->name;
    }
  }
  return $options;
}

/*
 * add custom WHERE clauses and joins to a query based on taxonomy.
 */
function views_handler_filter_tid_custom($op, $filter, $filterinfo, &$query) {
  // When filtering via depth, we have to add a chain. If it's an or query
  // we add 1 chain, but in an and query we actually basically have to add
  // a 2 dimensional array.

  _views_add_taxonomy($filter['operator'], $filter['value'], $filter['options'], $query);

}

/*
 * Create a list of vocabulary names and IDs.
 */
function views_handler_filter_vid() {
  $vids = array();
  $result = db_query("SELECT v.vid, v.name FROM {vocabulary} v ORDER BY v.weight, v.name");
  while ($obj = db_fetch_object($result)) {
    $vids[$obj->vid] = $obj->name;
  }
  return $vids;
}

/*
 * Add custom WHERE and JOIN clauses based on taxonomy vocabularies.
 */
function views_handler_filter_voc($op, $filter, $filterinfo, &$query) {
  _views_add_vocabulary($filter['operator'], $filter['value'], $filter['options'], $query);
}

/*
 * Add a where clause views_get_titled on taxonomy. This is a pretty exciting piece of
 * code because the clause and the JOINs are a bit complicated.
 */
function _views_add_taxonomy($op, $value, $depth, &$query) {
  // When filtering via depth, we have to add a chain. If it's an or query
  // we add 1 chain, but in an and query we actually basically have to add
  // a 2 dimensional array.

//  $query->ensure_table('term_data'); // this is to make sure the views_get_title table comes first.

  if ($op == 'OR') {
    $clause = "'" . implode("','", $value) . "'";
    // views_get_title tables

    // Any number of things might have already added a term_node table, but
    // if they have and are filtering on it the query is going to be borken
    // anyway. If it's been added and unfiltered, and we ignore it, we get
    // bad results.

    // However, if they added multiple or filters, we do have to separate
    // them a bit.
    if ($query->added_taxonomy_or) {
      $num = $query->add_table('term_node');
    }
    else {
      $num = 1;
      $query->ensure_table('term_node');
      $query->added_taxonomy_or = TRUE;
    }
    $tablename = $query->get_table_name('term_node', $num);
    $where = "$tablename.tid IN ($clause)";

    // for each depth > 0, add the next child in term_hierarchy to the join
    for ($i = 0; $i < $depth; $i++) {
      $thnum = $query->add_table('term_hierarchy', false, 1, array('left' => array('table' => $tablename, 'field' => 'tid'), 'right' => array('field' => 'tid')));
      $thname = $query->get_table_name('term_hierarchy', $thnum);
      $num = $query->add_table('term_data', false, 1, array('left' => array('table' => $thname, 'field' => 'parent'), 'right' => array('field' => 'tid')));
      $tablename = $query->get_table_name('term_data', $num);
      $where .= " OR $tablename.tid IN ($clause)";
    }
    $query->add_where("$where");
  }
  else {
    if ($op == 'NOR') {
      $not = "!";
    }
    foreach ($value as $tid) {
      // For every term we have to match, add a views_get_title table
      $num = 1;
      $num = $query->add_table('term_node');
      $tablename = $query->get_table_name('term_node', $num);
      $where = "$tablename.tid = '$tid'";

      // for each depth > 0, add the next child in term_hierarchy to the join
      for ($i = 0; $i < $depth; $i++) {
        $thnum = $query->add_table('term_hierarchy', false, 1, array('left' => array('table' => $tablename, 'field' => 'tid'), 'right' => array('field' => 'tid')));
        $thname = $query->get_table_name('term_hierarchy', $thnum);
        $num = $query->add_table('term_data', false, 1, array('left' => array('table' => $thname, 'field' => 'parent'), 'right' => array('field' => 'tid')));
        $tablename = $query->get_table_name('term_data', $num);
        $where .= " OR $tablename.tid = '$tid'";
      }
      $query->add_where("$not($where)");
    }
  }
}

/*
 * Add a WHERE clause views_get_title on taxonomy vocabulary
 */
function _views_add_vocabulary($op, $value, $depth, &$query) {
  if ($op == 'OR' || $op == 'NOR') {
    if ($op == 'NOR') {
      $not = 'NOT';
    }
    if (($op == 'NOR' || count($value) > 1) && count($query->fields)) {
      // group by views_title_table.views_title_field
      // count($query->fields) == 0 will break this code (can this happen?)
      $field = $query->fields[0];
      $query->add_groupby($field);
      $query->set_count_field("DISTINCT($field)");
    }
    $query->ensure_table('term_data');
    $clause = "'" . implode("','", $value) . "'";
    $query->add_where("term_data.vid $not IN ($clause)");
  }
  else {
    foreach ($value as $vid) {
      // For every vocabulary we have to match, add a views_get_title table
      $num = $query->add_table('term_node');
      $tablename = $query->get_table_name('term_node', $num);
      $num = $query->add_table('term_data', false, 1, array('left' => array('table' => $tablename, 'field' => 'tid'), 'right' => array('field' => 'tid')));
      $tablename = $query->get_table_name('term_data', $num);
      $query->add_where("$tablename.vid = '$vid'");
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
