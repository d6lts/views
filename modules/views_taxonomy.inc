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
      'option' => array(
         '#type' => 'select',
         '#options' => array(
           'link' => 'As links',
           'nolink' => 'Without links'
          ),
      ),
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
            'name' => t('Taxonomy: Terms for @voc-name', array('@voc-name' => $voc->name)),
            'sortable' => false,
            'help' => t('This will display all taxonomy terms associated with the node that are members of %voc-name. Note that this causes one extra query per row displayed, and might have a minor performance impact.', array('%voc-name' => $voc->name)),
            'handler' => 'views_handler_field_allterms',
            'vocabulary' => $voc->vid,
            'notafield' => true,
            'option' => array(
               '#type' => 'select',
               '#options' => array(
                 'link' => 'As links',
                 'nolink' => 'Without links'
                ),
            ),
          ),
        ),
        'filters' => array(
          'tid' => array(
            'name' => t('Taxonomy: Terms for @voc-name', array('@voc-name' => $voc->name)),
            'value' => views_taxonomy_form($voc),
            //views does handle this as array regardless what we state here due to the operators
            'value-type' => 'array', 
            'tags' => $voc->tags,
            'operator' => 'views_handler_operator_andor',
            'handler' => 'views_handler_filter_tid_custom',
            'option' => 'string',
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


function views_taxonomy_form(&$vocabulary) {
  if ($vocabulary->tags) {
    if ($vocabulary->help) {
      $help = $vocabulary->help;
    }
    else {
      $help = t('A comma-separated list of terms describing this content. Example: funny, bungee jumping, "Company, Inc.".');
    }
    $form = array('#type' => 'textfield',
      '#description' => $help,
      '#autocomplete_path' => 'taxonomy/autocomplete/'. $vocabulary->vid,
      '#process' => array('views_taxonomy_process_form' => array()),
      '#maxlength' => 255,
    );
  }
  else { 
    $form = taxonomy_form($vocabulary->vid, 0, $vocabulary->help);
    unset($form['#title']);
    unset($form['#options'][0]);
  }
  return $form;
}

/*
 * Due to the operators AND/OR/NOT views does handle this alway as array,
 * however the autocomplete field needs its string as default_value,
 * so this undoes the views default_value conversion for arrays in this case
 */
function views_taxonomy_process_form($element) {
  if ($element['#type'] == 'textfield' && is_array($element['#default_value'])) {
    $element['#default_value'] = implode(',', $element['#default_value']);
  }
  if ($element['#type'] == 'textfield' && is_array($element['#value'])) {
    $element['#value'] = implode(',', $element['#value']);
  }
  return $element;
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

  if ($fielddata['options'] == 'nolink') {
    foreach ($terms as $term) {
      $links[] = check_plain($term->name);
    }
  }
  else {
    $node = new stdClass();
    $node->taxonomy = $terms;
    $links = taxonomy_link('taxonomy terms', $node);
  }
  return !empty($links) ? theme('links', $links) : '';
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
  $term = taxonomy_get_term($data->$fieldname);
  $path = taxonomy_term_path($term);
  return l($value, $path);
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
      return $voc->name? check_plain($voc->name) : t('Uncatgorized');
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
  $result = db_query("SELECT DISTINCT(td.tid), td.name, td.weight, v.name as vocabname, v.weight FROM {term_data} td LEFT JOIN {vocabulary} v ON v.vid = td.vid $where ORDER BY v.weight, v.name, td.weight, td.name");
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

/*
 * add custom WHERE clauses and joins to a query based on taxonomy.
 */
function views_handler_filter_tid_custom($op, $filter, $filterinfo, &$query) {
  //if there is an autocomplete field, we have to transform the string values into the tids
  if ($filterinfo['tags']) {
    // This regexp allows the following types of user input:
    // this, "somecmpany, llc", "and ""this"" w,o.rks", foo bar
    $regexp = '%(?:^|,\ *)("(?>[^"]*)(?>""[^"]* )*"|(?: [^",]*))%x';
    preg_match_all($regexp, $filter['value'][0], $matches);
    $typed_terms = array_unique($matches[1]);
    
    $filter['value'] = array();
    foreach ($typed_terms as $typed_term) {
      // If a user has escaped a term (to demonstrate that it is a group,
      // or includes a comma or quote character), we remove the escape
      // formatting like taxonomye_node_save()
      $typed_term = str_replace('""', '"', preg_replace('/^"(.*)"$/', '\1', $typed_term));
      $typed_term = trim($typed_term);
      if ($typed_term == "") { continue; }

      // See if the term exists in the chosen vocabulary
      // and return the tid, otherwise, add a new record.
      $possibilities = taxonomy_get_term_by_name($typed_term);
      foreach ($possibilities as $possibility) {
        if ($possibility->vid == $filterinfo['vocabulary']) {
          $filter['value'][] = $possibility->tid;
        }
      }
    }
  }

  _views_add_taxonomy($filter['operator'], $filter['value'], $filter['options'], $query);

}

/*
 * Create a list of vocabulary names and IDs.
 */
function views_handler_filter_vid() {
  $vids = array();
  $result = db_query("SELECT v.vid, v.name, v.weight FROM {vocabulary} v ORDER BY v.weight, v.name");
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
  $value = array_map('intval', $value);
  // When filtering via depth, we have to add a chain. If it's an or query
  // we add 1 chain, but in an and query we actually basically have to add
  // a 2 dimensional array.

  if ($op == 'OR') {
    $num = $query->add_table('term_node');
    $tablename = $query->get_table_name('term_node', $num);
    $clause = "'" . implode("','", $value) . "'";
    $where = "$tablename.tid IN ($clause)";

    // for each depth > 0, add the next parent in term_hierarchy to the join
    $thnum = $query->add_table('term_hierarchy', false, 1, array('left' => array('table' => $tablename, 'field' => 'tid'), 'right' => array('field' => 'tid')));
    $tablename = $query->get_table_name('term_hierarchy', $thnum);
    for ($i = 0; $i < $depth; $i++) {
      $thnum = $query->add_table('term_hierarchy', false, 1, array('left' => array('table' => $tablename, 'field' => 'parent'), 'right' => array('field' => 'tid')));
      $tablename = $query->get_table_name('term_hierarchy', $thnum);
      $where .= " OR $tablename.tid IN ($clause)";
    }
    $query->add_where($where);
  }
  else if ($op == 'NOR') {
    //ignore the depth for this case
    $table_data = _views_get_tables();
    $joininfo = $table_data['term_node']['join'];
    $joininfo['extra']['tid'] = $value;
    $num = $query->add_table('term_node', false, 1, $joininfo);
    $tablename = $query->get_table_name('term_node', $num);
    $query->add_where("$tablename.tid IS NULL");
  }
  else {
    foreach ($value as $tid) {
      // For every term we have to match add the depth chain
      $num = $query->add_table('term_node');
      $tablename = $query->get_table_name('term_node', $num);
      $where = "$tablename.tid = '$tid'";

      // for each depth > 0, add the next parent in term_hierarchy to the join
      $thnum = $query->add_table('term_hierarchy', false, 1, array('left' => array('table' => $tablename, 'field' => 'tid'), 'right' => array('field' => 'tid')));
      $tablename = $query->get_table_name('term_hierarchy', $thnum);
      for ($i = 0; $i < $depth; $i++) {
        $thnum = $query->add_table('term_hierarchy', false, 1, array('left' => array('table' => $tablename, 'field' => 'parent'), 'right' => array('field' => 'tid')));
        $tablename = $query->get_table_name('term_hierarchy', $thnum);
        $where .= " OR $tablename.tid = '$tid'";
      }
      $query->add_where($where);
    }
  }
}

/*
 * Add a WHERE clause views_get_title on taxonomy vocabulary
 */
function _views_add_vocabulary($op, $value, $depth, &$query) {
  $value = array_map('intval', $value);

  if ($op == 'OR' || $op == 'NOR') {
    $num = $query->add_table('term_node');
    $tablename1 = $query->get_table_name('term_node', $num);
    $joininfo = array('left' => array('table' => $tablename1, 'field' => 'tid'), 'right' => array('field' => 'tid'));
    $joininfo['extra'] = array('vid' => $value);
    $num = $query->add_table('term_data', false, 1, $joininfo);
    $tablename2 = $query->get_table_name('term_data', $num);
    if ($op == 'OR') {
      $query->add_where("$tablename1.tid IS NOT NULL");
      $query->add_where("$tablename2.tid IS NOT NULL");
    }
    else {
      $query->add_where("($tablename1.tid IS NULL AND $tablename2.tid IS NULL)");
    }
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
