<?php

// ---------------------------------------------------------------------------
// Argument definitions.

/*
 * All of the arguments that Views supports natively. Some of this may
 * eventually be shipped off to the modules that provide this data.
 */
function _views_internal_arguments() {
  $arguments = array(
    'nodetype' => array(
      'name' => t('Node Type'),
      'handler' => 'views_handler_arg_nodetype',
      'help' => t('The node type argument allows users to filter a view by specifying the type of node.'),
    ),
    'uid' => array(
      'name' => t('User ID Authored'),
      'handler' => 'views_handler_arg_uid',
      'help' => t('The User ID argument allows users to filter to nodes authored by the specified user ID.'),
    ),
    'uidtouch' => array(
      'name' => t('User ID Authored or Commented'),
      'handler' => 'views_handler_arg_uid_touch',
      'help' => t('The User ID argument allows users to filter a to nodes authored or commented on the specified user ID.'),
    ),
    'taxid' => array(
      'name' => t('Taxonomy Term ID'),
      'handler' => 'views_handler_arg_taxid',
      'option' => 'string',
      'help' => t('The argument will filter by a taxonomy term ID. For this argument, set the option to the depth to search. See taxonomy for more information.'),
    ),
    'taxletter' => array(
      'name' => t('Taxonomy Term Name'),
      'handler' => 'views_handler_arg_taxletter',
      'option' => 'string',
      'help' => t('The argument will filter by a taxonomy term name. For this argument, set the option to the number of characters, using 0 for full term; use 1 for an A/B/C style glossary.'),
    ),
    'nodeletter' => array(
      'name' => t('Node Title'),
      'handler' => 'views_handler_arg_nodeletter',
      'option' => 'string',
      'help' => t('The argument will filter by the node title. For this argument, set the option to the number of characters, using 0 for full term; use 1 for an A/B/C style glossary.'),
    ),
    'year' => array(
      'name' => t('Year'),
      'handler' => 'views_handler_arg_year',
      'help' => t('This argument allows users to filter by what year the node was posted, in the form of CCYY.'),
    ),
    'month' => array(
      'name' => t('Month'),
      'handler' => 'views_handler_arg_month',
      'help' => t("Months are specified by the numbers 1-12. Since this argument does not specify a year, it is recommended that it usually follow a 'Year' argument."),
    ),
    'week' => array(
      'name' => t('Week'),
      'handler' => 'views_handler_arg_week',
      'help' => t("This allows the user to filter a view by the week number from 1-52. It is recommended this argument follow a 'Year' argument."),
    ),
    'monthyear' => array(
      'name' => t('Month + Year'),
      'handler' => 'views_handler_arg_monthyear',
      'help' => t('This argument combines Month and Year into a single argument, specified in the form CCYYMM.'),
    ),
    'fulldate' => array(
      'name' => t('Full Date'),
      'handler' => 'views_handler_arg_fulldate',
      'help' => t('This argument is a complete date in the form of CCYYMMDD.'),
    ),
    'nid' => array(
      'name' => t('Node ID'),
      'handler' => 'views_handler_arg_nid',
      'help' => t('This argument is a single Node ID.'),
    ),
  );
  return $arguments;
}

/*
 * Provides all of the internal table definition that Views supports natively.
 * This info will probably one day be shipped off to the modules that provide
 * it.
 */
function _views_internal_tables() {

  if (module_exist('book')) {
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
    );
  }
  
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
    ));

    $tables[$table->name] = $table;

    $vocabularies = taxonomy_get_vocabularies();
    foreach ($vocabularies as $voc) {
      $tables["term_node_$voc->name"] = array(
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
            'list' => 'views_handler_filter_tid',
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
      'name' => 'term_node',
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
          'name' => 'Taxonomy: Term',
          'sortable' => true,
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
          'help' => t('This will filter a view to only nodes that contain a term in the associated vocabulary.'),
        )
      )
    );
  }

  if (module_exist('comment')) {
    $tables['comments'] = array(
      'name' => 'comments',
      'provider' => 'internal',
      'join' => array(
        'left' => array(
          'table' => 'node',
          'field' => 'nid'
        ),
        'right' => array(
          'field' => 'nid'
        ),
      ),
    );

    $tables['node_comment_statistics'] = array(
      'name' => 'node_comment_statistics',
      'provider' => 'internal',
      'join' => array(
        'left' => array(
          'table' => 'node',
          'field' => 'nid'
        ),
        'right' => array(
          'field' => 'nid'
        ),
      ),
      'fields' => array(
        'last_comment_timestamp' => array(
          'name' => t('Comment: Last Comment Time'),
          'sortable' => true,
          'handler' => array(
            'views_handler_field_date_small' => t('As Short Date'),
            'views_handler_field_date'       => t('As Medium Date'),
            'views_handler_field_date_large' => t('As Long Date'),
            'views_handler_field_since'      => t('As Time Ago')
          ),
          'help' => t('This will display the last comment time.'),
         ),
        'last_comment_name' => array(
          'name' => t('Comment: Last Comment Author'),
          'handler' => 'views_handler_field_username',
          'sortable' => true,
          'uid' => 'last_comment_uid',
          'addlfields' => array('last_comment_uid'),
          'help' => t('This will display the name of the last user to comment on the post.'),
        ),
        'comment_count' => array(
          'name' => t('Comment: Count'),
          'sortable' => true,
          'handler' => array(
            'views_handler_field_int'         => t('Normal'),
            'views_handler_comments_with_new' => t('With New Count')
          ),
          'help' => t('This will display the comment count.'),
        ),
      ),
      'sorts' => array(
        'last_comment_timestamp' => array(
          'name' => t('Comment: Last Comment Date'),
          'help' => t('This will allow you to sort by the date of the most recent comment on a node.')
        )
      )
    );
  }

  $tables['users'] = array(
    'name' => 'users',
    'provider' => 'internal', // won't show up in external list.
    'join' => array(
      'left' => array(
        'table' => 'node',
        'field' => 'uid'
      ),
      'right' => array(
        'field' => 'uid'
      ),
    ),
    'fields' => array(
      'name' => array(
        'name' => t('Node: Author Name'),
        'handler' => 'views_handler_field_username',
        'sortable' => true,
        'uid' => 'uid',
        'addlfields' => array('uid'),
        'help' => t('This will display the author of the node.'),
      ),
    ),
    'sorts' => array(
      'name' => array(
        'name' => t('Node: Author Name'),
        'help' => t('This allows you to sort alphabetically by author.'),
      )
    ),
    'filters' => array(
      'uid' => array(
        'name' => t('Node: Author Name'),
        'operator' => 'views_handler_operator_or',
        'list' => 'views_handler_filter_username',
        'help' => t('This allows you to filter by a particular user. You might not find this useful if you have a lot of users.'),
      ),
    )
  );

  $tables['users_roles'] = array(
    'name' => 'users_roles',
    'provider' => 'internal', // won't show up in external list.
    'join' => array(
      'left' => array(
        'table' => 'node',
        'field' => 'uid'
      ),
      'right' => array(
        'field' => 'uid'
      ),
    ),
    'filters' => array(
      'rid' => array(
        'name' => t('Role: Author Role'),
        'operator' => 'views_handler_operator_andor',
        'list' => 'views_handler_filter_role',
        'help' => t('Include the node only if the author is a member of the selected role.'),
      ),
    )
  );

  // By default history table assumes current user.
  $tables['history'] = array(
    'name' => 'history',
    'provider' => 'internal',
    'join' => array(
      'left' => array(
        'table' => 'node',
        'field' => 'nid'
      ),
      'right' => array(
        'field' => 'nid'
      ),
      'extra' => array(
        'uid' => '***CURRENT_USER***'
      )
    ),
    'filters' => array(
      'timestamp' => array(
        'name' => t('Node: Has New Content'),
        'operator' => array('Has'),
        'list' => array('New Content'),
        'handler' => 'views_handler_filter_isnew',
        'help' => t('Including this filter will reduce the node set to nodes that have been updated or have new content since the user last read the node, as well as unread nodes.'),
      ),
    ),
  );

  $tables['node'] = array(
    'name' => 'node',
    'provider' => 'internal', // won't show up in external list.
    'fields' => array(
      'title' => array(
        'name' => t('Node: Title'),
        'handler' => array(
          'views_handler_field_nodelink'           => t('Normal'),
          'views_handler_field_nodelink_with_mark' => t('With updated mark')
        ),
        'sortable' => true,
        'addlfields' => array('changed'),
        'help' => t('Display the title of the node.'),
      ),
      'created' => array(
        'name' => t('Node: Created Time'),
        'sortable' => true,
        'handler' => array(
          'views_handler_field_date_small' => t('As Short Date'),
          'views_handler_field_date'       => t('As Medium Date'),
          'views_handler_field_date_large' => t('As Long Date'),
          'views_handler_field_since'      => t('As Time Ago'),
        ),
        'help' => t('Display the post time of the node.'),
      ),
      'changed' => array(
        'name' => t('Node: Updated Time'),
        'sortable' => true,
        'handler' => array(
          'views_handler_field_date_small' => t('As Short Date'),
          'views_handler_field_date'       => t('As Medium Date'),
          'views_handler_field_date_large' => t('As Long Date'),
          'views_handler_field_since'      => t('As Time Ago'),
        ),
        'help' => t('Display the last time the node was updated.'),
      ),
      'type' => array(
        'name' => t('Node: Type'),
        'handler' => 'views_handler_nodetype',
        'sortable' => true,
        'help' => t("The Node Type field will display the type of a node (for example, 'blog entry', 'forum post', 'story', etc)"),
      ),
    ),
    'sorts' => array(
      'created' => array(
        'name' => t('Node: Created Time'),
        'help' => t('Sort by the submission date of the node.'),
       ),
      'changed' => array(
        'name' => t('Node: Last Updated Time'),
        'help' => t('Sort by the last update date of the node.'),
      ),
      'sticky' => array(
        'name' => t('Node: Sticky'),
        'help' => t('Sort by whether or not the node is sticky. Choose descending to put sticky nodes at the top.'),
      ),
      'title' => array(
        'name' => t('Node: Title'),
        'help' => t('Sort by the node title, alphabetically'),
      ),
//      'random' => array(
//        'name' => t('Random'),
//        'handler' => 'views_handler_sort_random',
//        'help' => t('By choosing random, nodes will be ordered completely randomly. This is a good way to choose X random nodes from a group of nodes.'),
//      ),
    ),
    'filters' => array(
      'status' => array(
        'name' => t('Node: Published'),
        'operator' => array('=' => 'Equals'),
        'list' => 'views_handler_operator_yesno',
        'list-type' => 'select',
        'help' => t('Filter by whether or not the node is published. This is recommended for most Views!'),
      ),
      'promote' => array(
        'name' => t('Node: Front Page'),
        'operator' => array('=' => 'Equals'),
        'list' => 'views_handler_operator_yesno',
        'list-type' => 'select',
        'help' => t('Filter by whether or not the node has been promoted to Front Page.'),
      ),
      'moderate' => array(
        'name' => t('Node: Moderated'),
        'operator' => array('=' => 'Equals'),
        'list' => 'views_handler_operator_yesno',
        'list-type' => 'select',
        'help' => t('Filter by whether or not the node is moderated.'),
      ),
      'type' => array(
        'name' => t('Node: Type'),
        'list' => 'views_handler_filter_nodetype',
        'list-type' => 'list',
        'operator' => 'views_handler_operator_or',
        'help' => t('Include or exclude nodes of the selected types.'),
      ),
      'anon' => array(
        'field' => 'uid',
        'name' => t('Node: Author is Anonymous'),
        'operator' => 'views_handler_operator_eqneq',
        'list' => 'views_handler_filter_useranon',
        'help' => t('This allows you to filter by whether or not the node author is anonymous.'),
      ),
      'currentuid' => array(
        'field' => 'uid',
        'name' => t('Node: Author is Current User'),
        'operator' => 'views_handler_operator_eqneq',
        'list' => 'views_handler_filter_usercurrent',
        'list-type' => 'select',
        'help' => t('This allows you to filter by whether or not the node was authored by the logged in user of the view.'),
      ),
      'currentuidtouched' => array(
        'field' => 'uid',
        'name' => t('Node: Current User Authored or Commented'),
        'operator' => array('=' => 'touched by'),
        'list' => 'views_handler_filter_usercurrent',
        'list-type' => 'select',
        'handler' => 'views_handler_filter_uid_touched',
        'help' => t('This allows you to filter by whether or not the logged in user authored or commented on the node.'),
      ),
      'distinct' => array(
        'name' => t('Node: Distinct'),
        'operator' => array('=' => 'is'),
        'list' => array('distinct' => 'distinct'),
        'handler' => 'views_handler_filter_distinct',
        'help' => t('This filter ensures that each node may only be listed once, even if it matches multiple criteria. Use this if multiple taxonomy matches return duplicated nodes.'),
      ),
    ),
  );
  return $tables;
}

/*
 * Provides internal, default views that can always be used. This area is
 * a little grey right now -- many of these views should be provided
 * by other modules, and not always be active.
 */
function _views_internal_default_views() {
  $view = new stdClass();
  $view->name = 'frontpage';
  $view->description = 'The basic front page view.';
  $view->page = true;
  $view->url = 'frontpage';
  $view->page_title = '';
  $view->page_type = 'teaser';
  $view->use_pager = true;
  $view->nodes_per_page = variable_get('default_nodes_main', 10);
  $view->block = false;
  $view->menu = false;
  $view->breadcrumb_no_home = true;
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
  );
  $view->field = array (
  );
  $view->filter = array (
    array (
      'tablename' => 'node',
      'field' => 'promote',
      'operator' => '=',
      'options' => '',
      'value' => '1',
    ),
    array (
      'tablename' => 'node',
      'field' => 'status',
      'operator' => '=',
      'options' => '',
      'value' => '1',
    ),
  );
  $views[$view->name] = $view;


  $view = new stdClass();
  $view->name = 'tracker';
  $view->requires = array('node_comment_statistics');
  $view->description = 'Shows all new activity on system.';
  $view->page = true;
  $view->url = 'tracker';
  $view->page_title = 'recent posts for %1';
  $view->page_type = 'table';
  $view->use_pager = true;
  $view->nodes_per_page = '25';

  $view->menu = true;
  $view->menu_title = 'recent posts';
  $view->block = false;
  $view->field = array (
    array (
      'tablename' => 'node',
      'field' => 'type',
      'label' => 'Type',
    ),
    array (
      'tablename' => 'node',
      'field' => 'title',
      'label' => 'Title',
      'handler' => 'views_handler_field_nodelink_with_mark',
    ),
    array (
      'tablename' => 'users',
      'field' => 'name',
      'label' => 'Author',
    ),
    array (
      'tablename' => 'node_comment_statistics',
      'field' => 'comment_count',
      'label' => 'Replies',
      'handler' => 'views_handler_comments_with_new',
    ),
    array (
      'tablename' => 'node_comment_statistics',
      'field' => 'last_comment_timestamp',
      'label' => 'Last Post',
      'handler' => 'views_handler_field_since',
    ),
  );
  $view->sort = array (
    array (
      'tablename' => 'node_comment_statistics',
      'field' => 'last_comment_timestamp',
      'sortorder' => 'DESC',
    ),
  );
  $view->argument = array (
    array (
      'type' => 'uid',
      'argdefault' => '2',
      'title' => 'recent posts for all users',
    ),
  );
  $view->filter = array (
    array (
      'tablename' => 'node',
      'field' => 'status',
      'operator' => '=',
      'value' => '1',
    ),
  );

  $views[$view->name] = $view;

  return $views;
}

// ---------------------------------------------------------------------------
// View creator tool (for module developers)

/*
 * Get a true/false.
 */
function views_tf($val) {
  return ($val ? 'TRUE' : 'FALSE');
}

/*
 * This function creates view code for a view.
 */
function views_create_view_code($vid) {
  $view = _views_load_view($vid);
  if (!$view) {
    return "View '$vid' not found.";
  }

  $requires = array();

  $output = "  \$view = new stdClass();\n";
  $output .= "  \$view->name = '$view->name';\n";
  $output .= "  \$view->description = '$view->description';\n";
  $output .= "  \$view->access = " . var_export($view->access, true) . ";\n";
  // page items
  if ($view->page) {
    $output .= "  \$view->page = " . views_tf($view->page) . ";\n";
    $output .= "  \$view->page_title = '$view->page_title';\n";
    $output .= "  \$view->page_header = '$view->page_header';\n";
    $output .= "  \$view->page_header_format = '$view->page_header_format';\n";
    $output .= "  \$view->page_type = '$view->page_type';\n";
    $output .= "  \$view->url = '$view->url';\n";
    $output .= "  \$view->use_pager = " . views_tf($view->use_pager) . ";\n";
    $output .= "  \$view->nodes_per_page = '$view->nodes_per_page';\n";
    if ($view->menu) {
      $output .= "  \$view->menu = " . views_tf($view->menu) . ";\n";
      $output .= "  \$view->menu_title = '$view->menu_title';\n";
      $output .= "  \$view->menu_tab = " . views_tf($view->menu_tab) . ";\n";
      $output .= "  \$view->menu_tab_default = " . views_tf($view->menu_tab_default) . ";\n";
      $output .= "  \$view->menu_weight = '$view->menu_weight';\n";
    }
  }
  if ($view->block) {
    $output .= "  \$view->block = " . views_tf($view->block) . ";\n";
    $output .= "  \$view->block_title = '$view->block_title';\n";
    $output .= "  \$view->block_header = '$view->block_header';\n";
    $output .= "  \$view->block_header_format = '$view->block_header_format';\n";
    $output .= "  \$view->block_type = '$view->block_type';\n";
    $output .= "  \$view->nodes_per_block = '$view->nodes_per_block';\n";
    $output .= "  \$view->block_more = '$view->block_more';\n";
    $output .= "  \$view->block_use_page_header = " . views_tf($view->block_use_page_header) . ";\n";
  }

  $output .= "  \$view->sort = array (\n";
  foreach ($view->sort as $sort) {
    $output .= "    array (\n";
    $fieldbits = explode('.', $sort['field']);
    $output .= "      'tablename' => '$fieldbits[0]',\n";
    $output .= "      'field' => '$fieldbits[1]',\n";
    $output .= "      'sortorder' => '$sort[sortorder]',\n";
    $output .= "      'options' => '$sort[options]',\n";
    $output .= "    ),\n";
    $requires[$fieldbits[0]] = 1;
  }
  $output .= "  );\n";

  $output .= "  \$view->argument = array (\n";
  foreach ($view->argument as $argument) {
    $output .= "    array (\n";
    $output .= "      'type' => '$argument[type]',\n";
    $output .= "      'argdefault' => '$argument[argdefault]',\n";
    $output .= "      'title' => '$argument[title]',\n";
    $output .= "      'options' => '$argument[options]',\n";
    $output .= "    ),\n";
  }
  $output .= "  );\n";

  $output .= "  \$view->field = array (\n";
  foreach ($view->field as $field) {
    $output .= "    array (\n";
    $output .= "      'tablename' => '$field[tablename]',\n";
    $output .= "      'field' => '$field[field]',\n";
    $output .= "      'label' => '$field[label]',\n";
    if ($field['handler']) {
      $output .= "      'handler' => '$field[handler]',\n";
    }
    if ($field['sortable']) {
      $output .= "      'sortable' => '$field[sortable]',\n";
    }
    if ($field['defaultsort']) {
      $output .= "      'defaultsort' => '$field[defaultsort]',\n";
    }
    $output .= "    ),\n";
    $requires[$field['tablename']] = 1;
  }
  $output .= "  );\n";

  $output .= "  \$view->filter = array (\n";
  foreach ($view->filter as $filter) {
    $output .= "    array (\n";
    $fieldbits = explode('.', $filter['field']);
    $output .= "      'tablename' => '$fieldbits[0]',\n";
    $output .= "      'field' => '$fieldbits[1]',\n";
    $output .= "      'operator' => '$filter[operator]',\n";
    $value = var_export($filter['value'], true);
    $output .= "      'options' => '$filter[options]',\n";
    $output .= "      'value' => $value,\n";
    $output .= "    ),\n";
    $requires[$fieldbits[0]] = 1;
  }
  $output .= "  );\n";
  $output .= "  \$view->requires = array(" . implode(', ', array_keys($requires)) . ");\n";
  $output .= "  \$views[\$view->name] = \$view;\n";
  // no breadcrumb for now.
  return $output;
}
