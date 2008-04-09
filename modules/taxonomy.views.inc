<?php
/**
 * @file
 *
 * Provide views data and handlers for taxonomy.module
 */

/**
 * @defgroup views_taxonomy_module taxonomy.module handlers
 *
 * @{
 */

/**
 * Implementation of hook_views_data()
 */
function taxonomy_views_data() {
//  $vocabularies = taxonomy_get_vocabularies();

  $data = array();

  // ----------------------------------------------------------------------
  // vocabulary table

  $data['vocabulary']['table']['group']  = t('Taxonomy');

  $data['vocabulary']['table']['join'] = array(
    // vocabulary links to term_data directly via vid.
    'term_data' => array(
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    // vocabulary links to node through term_data via vid
    'node' => array(
      'left_table' => 'term_data',
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    // vocabulary links to node_revision via term_data
    'node_revision' => array(
      'left_table' => 'term_data',
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    // vocabulary links to users via term_data
    'users' => array(
      'left_table' => 'term_data',
      'left_field' => 'vid',
      'field' => 'vid',
    ),
  );

  // vocabulary name
  $data['vocabulary']['name'] = array(
    'title' => t('Vocabulary name'), // The item it appears as on the UI,
    'field' => array(
      'help' => t('Name of the vocabulary a term is a member of. This will be the vocabulary that whichever term the "Taxonomy: Term" field is; and can similarly cause duplicates.'),
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
  );
  $data['vocabulary']['vid'] = array(
    'title' => t('Vocabulary ID'), // The item it appears as on the UI,
    'help' => t('The taxonomy vocabulary ID'),
    'argument' => array(
      'handler' => 'views_handler_argument_vocabulary_vid',
      'name field' => 'name',
    ),
  );

  // ----------------------------------------------------------------------
  // term_data table

  $data['term_data']['table']['group']  = t('Taxonomy');
  $data['term_data']['table']['base'] = array(
    'field' => 'tid',
    'title' => t('Terms'),
    'help' => t('Taxonomy terms are attached to nodes.'),
    'weight' => 10,
  );


  // The term data table
  $data['term_data']['table']['join'] = array(
    'node' => array(
      'left_table' => 'term_node',
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'node_revision' => array(
      'left_table' => 'term_node',
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'users' => array(
      'left_table' => 'term_node',
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    // This is provided for many_to_one argument
    'term_node' => array(
      'field' => 'tid',
      'left_field' => 'tid',
    ),
  );

  // Term name field
  $data['term_data']['name'] = array(
    'title' => t('Term'),
    'help' => t('Taxonomy terms. Note that using this can cause duplicate nodes to appear in views; you must add filters to reduce the resultset.'),
    'field' => array(
      'handler' => 'views_handler_field_taxonomy',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string', // @todo
      'help' => t('Taxonomy term name'),
      'many to one' => TRUE,
      'empty name field' => t('Uncategorized'),
    ),
  );

  // taxonomy weight
  $data['term_data']['weight'] = array(
    'title' => t('Weight'),
    'help' => t('The term weight field'),
    'field' => array(
      'handler' => 'views_hander_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_hander_sort',
    ),
  );

  // Term description
  $data['term_data']['description'] = array(
    'title' => t('Term description'), // The item it appears as on the UI,
    'help' => t('The description associated with a taxonomy term.'),
    'field' => array(
      'field' => 'description', // the real field
      'group' => t('Taxonomy'), // The group it appears in on the UI,
      'handler' => 'views_handler_field_markup',
      'format' => FILTER_FORMAT_DEFAULT,
    ),
  );

  // Term vocabulary
  $data['term_data']['vid'] = array(
    'title' => t('Vocabulary'),
    'help' => t('Filter the results of "Taxonomy: Term" to a particular vocabulary.'),
    'filter' => array(
      'handler' => 'views_handler_filter_vocabulary_vid', // @todo
    ),
  );

  // ----------------------------------------------------------------------
  // term_node table

  $data['term_node']['table']['group']  = t('Taxonomy');

  $data['term_node']['table']['join'] = array(
    'term_data' => array(
      // links directly to term_data via tid
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'node' => array(
      // links directly to node via nid
      'left_field' => 'nid',
      'field' => 'nid',
    ),
    'node_revisions' => array(
      // links directly to node_revisions via vid
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    'users' => array(
      // Links to users via node.nid
      'left_table' => 'node',
      'left_field' => 'nid',
      'field' => 'nid',
    ),
    'term_hierarchy' => array(
      'left_field' => 'tid',
      'field' => 'tid',
    ),
  );

  // tid field
  $data['term_node']['tid'] = array(
    'title' => t('Term ID'),
    'help' => t('The taxonomy term ID'),
    'field' => array(
      'title' => t('All terms'),
      'help' => t('Display all taxonomy terms associated with a node.'),
      'handler' => 'views_handler_field_term_node_tid',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_term_node_tid',
      'name table' => 'term_data',
      'name field' => 'name',
      'empty name field' => t('Uncategorized'),
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_term_node_tid',
      'hierarchy table' => 'term_hierarchy',
      'numeric' => TRUE,
    ),
  );

  // @todo: term_relation
  // @todo: term_synonym

  // ----------------------------------------------------------------------
  // term_hierarchy table

  $data['term_hierarchy']['table']['group']  = t('Taxonomy');

  $data['term_hierarchy']['table']['join'] = array(
    'term_hierarchy' => array(
      // links to self through left.parent = right.tid (going down in depth)
      'left_field' => 'tid',
      'field' => 'parent',
    ),
    'term_data' => array(
      // links directly to term_data via tid
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'node' => array(
      // links to node thorugh term_data
      'left_table' => 'term_data',
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'node_revisions' => array(
      // links to node_revisions thorugh term_data
      'left_table' => 'term_data',
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'users' => array(
      // links to users thorugh term_data
      'left_table' => 'term_data',
      'left_field' => 'tid',
      'field' => 'tid',
    ),
  );

  // ----------------------------------------------------------------------
  // term_synonym table

  $data['term_synonym']['table']['group']  = t('Taxonomy');

  $data['term_synonym']['table']['join'] = array(
    'term_data' => array(
      // links directly to term_data via tid
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'node' => array(
      'left_table' => 'term_node',
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'node_revisions' => array(
      'left_table' => 'term_node',
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'users' => array(
      // Links to users via node
      'left_table' => 'term_node',
      'left_field' => 'tid',
      'field' => 'tid',
    ),
  );

  $data['term_synonym']['name'] = array(
    'title' => t('Term synonym'),
    'help' => t('Term synonyms may be used to find terms by alternate names'),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
      'many to one' => TRUE,
      'empty name field' => t('Uncategorized'),
    ),
  );
  return $data;
}

class views_handler_field_term_node_tid extends views_handler_field_prerender_list {
  function init(&$view, $options) {
    parent::init($view, $options);
    if ($view->base_table == 'node_revisions') {
      $this->additional_fields['nid'] = array('table' => 'node_revisions', 'field' => 'vid');
    }
    else {
      $this->additional_fields['nid'] = array('table' => 'node', 'field' => 'nid');
    }
  }

  /**
   * Provide meaningful defaults
   */
  function options(&$options) {
    parent::options($options);
    $options['link_to_taxonomy'] = TRUE;
    $options['limit'] = FALSE;
    $options['vids'] = array();
  }

  /**
   * Provide "link to term" option.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_to_taxonomy'] = array(
      '#title' => t('Link this field to its term page'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['link_to_taxonomy']),
    );

    $form['limit'] = array(
      '#type' => 'checkbox',
      '#title' => t('Limit terms by vocabulary'),
      '#default_value'=> $this->options['limit'],
    );

    $options = array();
    $vocabularies = taxonomy_get_vocabularies();
    foreach ($vocabularies as $voc) {
      $options[$voc->vid] = $voc->name;
    }

    $form['vids'] = array(
      '#prefix' => '<div><div id="edit-options-vids">',
      '#suffix' => '</div></div>',
      '#type' => 'checkboxes',
      '#title' => t('Vocabularies'),
      '#options' => $options,
      '#default_value' => $this->options['vids'],
      '#process' => array('expand_checkboxes', 'views_process_dependency'),
      '#dependency' => array('edit-options-limit' => array(TRUE)),
    );
  }

  /**
   * Add this term to the query
   */
  function query() {
    $this->add_additional_fields();
  }

  function pre_render($results) {
    $this->field_alias = $this->aliases['nid'];
    $nids = array();
    foreach ($results as $result) {
      $nids[] = $result->{$this->aliases['nid']};
    }

    if ($nids) {
      $vids = '';
      if (!empty($this->options['limit']) && !empty($this->options['vids'])) {
        $vids = " AND td.vid IN (" . implode(', ', array_keys(array_filter($this->options['vids']))) . ")";
      }

      $result = db_query("SELECT tn.nid, td.* FROM {term_data} td INNER JOIN {term_node} tn ON td.tid = tn.tid WHERE tn.nid IN (" . implode(', ', $nids) . ")$vids ORDER BY td.weight, td.name");

      while ($term = db_fetch_object($result)) {
        if (empty($this->options['link_to_taxonomy'])) {
          $this->items[$term->nid][$term->tid] = check_plain($term->name);
        }
        else {
          $this->items[$term->nid][$term->tid] = l($term->name, taxonomy_term_path($term));
        }
      }
    }
  }
}

/**
 * Field handler to provide simple renderer that allows linking to a taxonomy
 * term.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_taxonomy extends views_handler_field {
  /**
   * Constructor to provide additional field to add.
   *
   * This constructer assumes the term_data table. If using another
   * table, we'll need to be more specific.
   */
  function construct() {
    parent::construct();
    $this->additional_fields['vid'] = 'vid';
    $this->additional_fields['tid'] = 'tid';
  }

  /**
   * Provide link to taxonomy option
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_to_taxonomy'] = array(
      '#title' => t('Link this field to its taxonomy term page'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['link_to_taxonomy']),
    );
  }

  /**
   * Render whatever the data is as a link to the taxonomy.
   *
   * Data should be made XSS safe prior to calling this function.
   */
  function render_link($data, $values) {
    if (!empty($this->options['link_to_taxonomy']) && !empty($values->{$this->aliases['tid']})) {
      $term = new stdClass();
      $term->tid = $values->{$this->aliases['tid']};
      $term->vid = $values->{$this->aliases['vid']};

      return l($data, taxonomy_term_path($term), array('html' => TRUE));
    }
    else {
      return $data;
    }
  }

  function render($values) {
    return $this->render_link(check_plain($values->{$this->field_alias}), $values);
  }
}

class views_handler_argument_taxonomy extends views_handler_argument {

  /**
   * Override the behavior of title(). Get the title of the node.
   */
  function title() {
    $term = taxonomy_get_term($this->argument);
    if (!empty($term)) {
      return check_plain($term->name);
    }
    // TODO review text
    return t('No name');
  }
}

/**
 * Filter by vocabulary id
 *
 * @ingroup views_filter_handlers
 */
class views_handler_filter_vocabulary_vid extends views_handler_filter_in_operator {
  function get_value_options() {
    if (isset($this->value_options)) {
      return;
    }

    $this->value_options = array();
    $vocabularies = taxonomy_get_vocabularies();
    foreach ($vocabularies as $voc) {
      $this->value_options[$voc->vid] = $voc->name;
    }
  }
}

/**
 * Argument handler to accept a user id.
 *
 * @ingroup views_argument_handlers
 */
class views_handler_argument_vocabulary_vid extends views_handler_argument {
  /**
   * Override the behavior of title(). Get the name of the user.
   */
  function title() {
    $title = db_result(db_query("SELECT v.name FROM {vocabulary} v WHERE v.vid = %d", $this->argument));

    if (empty($title)) {
      return t('No vocabulary');
    }

    return check_plain($title);
  }
}

/**
 * Filter by term id
 *
 * @ingroup views_filter_handlers
 */
class views_handler_filter_term_node_tid extends views_handler_filter_many_to_one {
  function has_extra_options() { return TRUE; }

  function get_value_options() { /* don't overwrite the value options */ }

  function options(&$options) {
    parent::options($options);
    $options['type'] = 'textfield';
    $options['vid'] = db_result(db_query('SELECT MIN(vid) FROM {vocabulary} v'));
  }

  function extra_options_form(&$form, &$form_state) {
    $vocabularies = taxonomy_get_vocabularies();
    foreach ($vocabularies as $voc) {
      $options[$voc->vid] = $voc->name;
    }

    $form['vid'] = array(
      '#prefix' => '<div class="views-left-40">',
      '#suffix' => '</div>',
      '#type' => 'radios',
      '#title' => t('Vocabulary'),
      '#options' => $options,
      '#description' => t('Select which vocabulary to show terms for in the regular options'),
      '#default_value' => $this->options['vid'],
    );

    $form['type'] = array(
      '#prefix' => '<div class="views-left-40">',
      '#suffix' => '</div>',
      '#type' => 'radios',
      '#title' => t('Selection type'),
      '#options' => array('select' => t('Dropdown'), 'textfield' => t('Autocomplete')),
      '#default_value' => $this->options['type'],
    );

  }

  function value_form(&$form, &$form_state) {
    $vocabulary = taxonomy_vocabulary_load($this->options['vid']);
    if (!$vocabulary) {
      $form['markup'] = array(
        '#prefix' => '<div class="form-item">',
        '#suffix' => '</div>',
        '#value' => t('An invalid vocabulary is selected. Please change it in the options.'),
      );
      return;
    }

    if ($this->options['type'] == 'textfield') {
      $default = '';
      if ($this->value) {
        $result = db_query("SELECT * FROM {term_data} td WHERE td.tid IN (" . implode(', ', $this->value) . ')');
        while ($term = db_fetch_object($result)) {
          if ($default) {
            $default .= ', ';
          }
          $default .= $term->name;
        }
      }

      $form['value'] = array(
        '#title' => t('Select terms from vocabulary @voc', array('@voc' => $vocabulary->name)),
        '#type' => 'textfield',
        '#autocomplete_path' => 'taxonomy/autocomplete/' . $vocabulary->vid,
        '#default_value' => $default,
      );
    }
    else {
      $form['value'] = _taxonomy_term_select(
        t('Select terms from vocabulary @voc', array('@voc' => $vocabulary->name)),
        'value',
        $this->value,
        $vocabulary->vid,
        '',
        TRUE,
        NULL
      );
      unset($form['value']['#weight']);
    }

    // Retain the helper option
    $this->helper->options_form($form, $form_state);
  }

  function value_validate($form, &$form_state) {
    if ($this->options['type'] == 'textfield') {
      $values = drupal_explode_tags($form_state['values']['options']['value']);
      if (empty($values)) {
        $form_state['values']['options']['value'] = array();
        return;
      }

      $tids = array();
      $placeholders = array();
      $args = array();
      $results = array();
      foreach ($values as $value) {
        if (strtolower($value) == 'anonymous') {
          $tids[] = 0;
        }
        else {
          $missing[strtolower($value)] = TRUE;
          $args[] = $value;
          $placeholders[] = "'%s'";
        }
      }

      $result = db_query("SELECT * FROM {term_data} WHERE name IN (" . implode(', ', $placeholders) . ")", $args);
      while ($term = db_fetch_object($result)) {
        unset($missing[strtolower($term->name)]);
        $tids[] = $term->tid;
      }

      if ($missing) {
        form_error($form['value'], t('Unable to find term(s): @terms', array('@terms', implode(', ', $missing))));
      }

      $form_state['values']['options']['value'] = $tids;
    }
  }

  function value_submit($form, &$form_state) {
    // prevent array_filter from messing up our arrays in parent submit.
  }

  function admin_summary() {
    // set up $this->value_options for the parent summary
    $this->value_options = array();

    if ($this->value) {
      $result = db_query("SELECT * FROM {term_data} td WHERE td.tid IN ("  . implode(', ', $this->value) . ")");

      while ($term = db_fetch_object($result)) {
        $this->value_options[$term->tid] = $term->name;
      }
    }
    return parent::admin_summary();
  }
}

/**
 * Allow taxonomy term ID(s) as argument
 *
 * @ingroup views_argument_handlers
 */
class views_handler_argument_term_node_tid extends views_handler_argument_many_to_one {
  function title_query() {
    $titles = array();
    $tids = implode(', ', $this->value); // only does numbers so safe

    $result = db_query("SELECT name FROM {term_data} WHERE tid IN (%s)", $tids);
    while ($term = db_fetch_object($result)) {
      $titles[] = check_plain($term->name);
    }
    return $titles;
  }
}

/**
 * @}
 */
