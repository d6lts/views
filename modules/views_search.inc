<?php
// $Id$

function search_views_tables() {
  $tables['temp_search_results'] = array(
    'name' => 'temp_search_results',
    'join' => array(
      'left' => array(
        'table' => 'node',
        'field' => 'nid',
      ),
      'right' => array(
        'field' => 'sid',
      ),
    ),
    'filters' => array(
      'word' => array(
        'name' => t('Search: Index'),
        'operator' => array('=' => t('Contains')),
        'handler' => 'search_views_handler_search_index',
      ),
    ),
  );
  return $tables;
}

function search_views_handler_search_index($op, $filter, $filterdata, &$query) {
  switch ($op) {
    case 'handler':
      $select2 = 'i.relevance AS score';
      $search = search_parse_query($filter['value']);
      if ($search === NULL || $search[0] == '' || $search[2] == '') {
        return;
      }

      $conditions = '('. $search[2] .") AND i.type = 'node'";
      $arguments = array_merge($search[3], array($search[4]));
      $result = db_query_temporary("SELECT i.type, i.sid, SUM(i.score * t.count) AS relevance, COUNT(*) AS matches FROM {search_index} i INNER JOIN {search_total} t ON i.word = t.word $join1 WHERE $conditions GROUP BY i.type, i.sid HAVING COUNT(*) >= %d", $arguments, 'temp_search_sids');

      // Calculate maximum relevance, to normalize it
      $normalize = db_result(db_query('SELECT MAX(relevance) FROM temp_search_sids'));
      if (!$normalize) {
        $query->add_where("FALSE");
        return;
      }
      $select2 = str_replace('i.relevance', '('. (1.0 / $normalize) .' * i.relevance)', $select2);

      // Second pass: only keep items that match the complicated keywords conditions (phrase search, negative keywords, ...)
      $conditions = '('. $search[0] .')';
      $arguments = $search[1];
      $result = db_query_temporary("SELECT i.type, i.sid, $select2 FROM temp_search_sids i INNER JOIN {search_dataset} d ON i.sid = d.sid AND i.type = d.type $join2 WHERE $conditions $sort_parameters", $arguments, 'temp_search_results');
      if (($count = db_result(db_query('SELECT COUNT(*) FROM temp_search_results'))) == 0) {
        $query->add_where("0");
        return;
      }

      $query->ensure_table('temp_search_results');
      $query->add_where('temp_search_results.sid IS NOT NULL');
  }
}
