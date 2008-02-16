<?php
// $Id$
/**
 * @file
 * Provide views data and handlers for node.module
 */

/**
 * @defgroup views_node_module node.module handlers
 *
 * Includes the tables 'node', 'node_revisions' and 'history'.
 * @{
 */

/**
 * Implementation of hook_views_data()
 */
function node_views_data() {
  // Basic table information.

  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.
  $data['node']['table']['group']  = t('Node');

  // Advertise this table as a possible base table
  $data['node']['table']['base'] = array(
    'field' => 'nid',
    'title' => t('Node'),
    'help' => t('The node type allows views of your site\'s primary content.'),
    'weight' => 10,
  );

  // For other base tables, explain how we join
  $data['node']['table']['join'] = array(
    'users' => array(
      'handler' => 'views_join', // this is actually optional
      'arguments' => array('node', 'users', 'uid', 'uid'),
     ),
    'node_revisions' => array(
      'handler' => 'views_join', // this is actually optional
      'arguments' => array('node', 'node_revisions', 'nid', 'nid'),
     ),
  );

  // ----------------------------------------------------------------
  // Fields

  // title
  $data['node']['title'] = array(
    'title' => t('Title'), // The item it appears as on the UI,
    'help' => t('The title of the node.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'field' => 'title', // the real field
      'group' => t('Node'), // The group it appears in on the UI,
      'handler' => 'views_handler_field_node',
      'click sortable' => TRUE,
     ),
  );

  // nid
  $data['node']['nid'] = array(
    'title' => t('Nid'),
    'help' => t('The node ID of the node'), // The help that appears on the UI,
    // Information for displaying the nid
    'field' => array(
      'handler' => 'views_handler_field_node',
      'click sortable' => TRUE,
    ),
    // Information for accepting a nid as an argument
    'argument' => array(
      'handler' => 'views_handler_argument_node_nid',
      'click sortable' => TRUE,
    ),
    // Information for accepting a nid as a filter
    'filter' => array(
      'handler' => 'views_handler_filter_equality',
    ),
    // Information for sorting on a nid.
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // created field
  $data['node']['created'] = array(
    'title' => t('Post date'), // The item it appears as on the UI,
    'help' => t('The date the node was posted.'), // The help that appears on the UI,
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // changed field
  $data['node']['changed'] = array(
    'title' => t('Updated date'), // The item it appears as on the UI,
    'help' => t('The date the node was last updated.'), // The help that appears on the UI,
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
  );

  // Node type
  $data['node']['type'] = array(
    'title' => t('Type'), // The item it appears as on the UI,
    'help' => t('The type of a node (for example, "blog entry", "forum post", "story", etc)'), // The help that appears on the UI,
    'field' => array(
      'handler' => 'views_handler_field_node_type',
      'click sortable' => TRUE,
    ),
    // Information for accepting a nid as a filter
    'filter' => array(
      'handler' => 'views_handler_filter_node_type',
    ),    
    'argument' => array(
      'handler' => 'views_handler_argument_node_type',
    ),
  );

  // published status
  $data['node']['status'] = array(
    'title' => t('Published'), // The item it appears as on the UI,
    'help' => t('The published status of the node.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
     ),
     'filter' => array(
       'handler' => 'views_handler_filter_boolean_operator',
       'label' => t('Published'),
       ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // promote status
  $data['node']['promote'] = array(
    'title' => t('Promoted to front page'), // The item it appears as on the UI,
    'help' => t('The front page of the node.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
     ),
     'filter' => array(
       'handler' => 'views_handler_filter_boolean_operator',
       'label' => t('Promoted to front page'),
       ),     
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // sticky
  $data['node']['sticky'] = array(
    'title' => t('Sticky'), // The item it appears as on the UI,
    'help' => t('Whether or not the node is sticky.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
     ),
     'filter' => array(
       'handler' => 'views_handler_filter_boolean_operator',
       'label' => t('Sticky'),
       ),     
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // Bogus fields for aliasing purposes.

  $data['node']['created_year_month'] = array(
    'title' => t('Created year + month'),
    'help' => t('In the form of YYYYMM'),
    'argument' => array(
      'handler' => 'views_handler_argument_node_created_year_month',
    ),
  );

  $data['node']['created_year'] = array(
    'title' => t('Created year'),
    'help' => t('In the form of YYYY'),
    'argument' => array(
      'handler' => 'views_handler_argument_node_created_year',
    ),
  );

  $data['node']['created_month'] = array(
    'title' => t('Created month'),
    'help' => t('In the form of MM 01 - 12'),
    'argument' => array(
      'handler' => 'views_handler_argument_node_created_month',
    ),
  );

  // ----------------------------------------------------------------------
  // Node revisions table

  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.
  $data['node_revisions']['table']['group']  = t('Node');

  // Advertise this table as a possible base table
  $data['node_revisions']['table']['base'] = array(
    'field' => 'vid',
    'title' => t('Node revisions'),
    'help' => t('Node revisions allow you to list revisions to nodes.'),
  );

  // For other base tables, explain how we join
  $data['node_revisions']['table']['join'] = array(
    'node' => array(
      'handler' => 'views_join', // this is actually optional
      'arguments' => array('node_revisions', 'node', 'vid', 'vid'),
     ),
    'user' => array(
      'handler' => 'views_join', // this is actually optional
      'arguments' => array('node_revisions', 'node', 'vid', 'vid'),
     ),
  );

  // Body field
  $data['node_revisions']['body'] = array(
    'title' => t('Body'), // The item it appears as on the UI,
    'help' => t('The actual, full data in the body field; this may not be valid data on all node types.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_markup',
      'format' => 'format', // The name of the format field
     ),
  );

  // Teaser field
  $data['node_revisions']['teaser'] = array(
    'title' => t('Teaser'), // The item it appears as on the UI,
    'help' => t('The stored teaser field. This may not be valid or useful data on all node types.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_markup',
      'format' => 'format', // The name of the format field
     ),
  );

  return $data;
}

/**
 * Field handler to provide simple renderer that allows linking to a node.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_node extends views_handler_field {
  /**
   * Override init function to provide generic option to link to node.
   */
  function init(&$view, &$data) {
    parent::init($view, $data);
    if (isset($data->data->link_to_node) && $view->base_table != 'node') {
      // @todo: This can't be right, it'll totally fail with relationships.
      $this->additional_fields[] = 'nid';
      $this->nid_field = 'node_nid';
    }
    else {
      $this->nid_field = 'nid';
    }
  }

  /**
   * Provide link to node option
   */
  function options_form(&$form, &$form_state) {
    $form['link_to_node'] = array(
      '#title' => t('Link this field to its node'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->data->link_to_node),
    );
  }

  function render_link($data, $values) {
    if (!empty($this->data->link_to_node)) {
      return l($data, "node/" . $values->{$this->nid_field}, array('html' => TRUE));
    }
    else {
      return $data;
    }
  }

  function render($values) {
    return $this->render_link(check_plain($values->{$this->field_alias}), $values);
  }
}

/**
 * Field handler to translate a node type into its readable form.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_node_type extends views_handler_field_node {
  function render($values) {
    $value = node_get_types('name', $values->{$this->field_alias});
    return $this->render_link(check_plain($value), $values);
  }
}

/**
 * Argument handler to accept a node type.
 *
 * @ingroup views_argument_handlers
 */
class views_handler_argument_node_type extends views_handler_argument {
  function construct() {
    parent::construct('type');
  }

  /**
   * Override the behavior of summary_name(). Get the user friendly version
   * of the node type.
   */
  function summary_name($data) {
    return $this->node_type($data->{$this->name_alias});
  }

  /**
   * Override the behavior of title(). Get the user friendly version of the
   * node type.
   */
  function title() {
    return $this->node_type($this->argument);
  }

  function node_type($type) {
    $output = node_get_types('name', $type);
    if (empty($output)) {
      $output = t('Unknown node type');
    }
    return check_plain($output);
  }
}

/**
 * Argument handler to accept a node id.
 *
 * @ingroup views_argument_handlers
 */
class views_handler_argument_node_nid extends views_handler_argument {
  // No constructor is necessary.

  /**
   * Override the behavior of title(). Get the title of the node.
   */
  function title() {
    $title = db_result(db_query(db_rewrite_sql("SELECT n.title FROM {node} n WHERE n.nid = %d"), $this->argument));
    if (empty($title)) {
      return t('No title');
    }

    return check_plain($title);
  }
}

/**
 * Argument handler for a year (CCYY)
 */
class views_handler_argument_node_created_year extends views_handler_argument_formula {
  /**
   * Constructor implementation
   *
   * @todo node.created should instead know about table aliases otherwise this
   * will explode in relationships.
   */
  function construct() {
    $timezone = views_get_timezone();
    $this->formula = "YEAR(FROM_UNIXTIME(node.created+$timezone))";
  }

  /**
   * Provide a link to the next level of the view
   */
  function summary_link($data, $url) {
    $value = $data->{$this->base_alias};
    return l($value, "$url/$value");
  }
}

/**
 * Argument handler for a year plus month (CCYYMM)
 */
class views_handler_argument_node_created_year_month extends views_handler_argument_formula {
  /**
   * Constructor implementation
   */
  function construct() {
    $timezone = views_get_timezone();
    $this->formula =  "DATE_FORMAT(FROM_UNIXTIME(node.created+$timezone), '%Y%m')";
    $this->format = 'F, Y';
  }

  /**
   * Provide a link to the next level of the view
   */
  function summary_name($data) {
    $created = $data->{$this->name_alias};
    return format_date(strtotime($created . "15"), 'custom', $this->format, 0);
  }

  /**
   * Provide a link to the next level of the view
   */
  function title($data, $url) {
    return format_date(strtotime($this->argument . "15"), 'custom', $this->format, 0);
  }
}

/**
 * Argument handler for a month (MM)
 */
class views_handler_argument_node_created_month extends views_handler_argument_formula {
  /**
   * Constructor implementation
   */
  function construct() {
    $timezone = views_get_timezone();
    $this->formula =  "MONTH(FROM_UNIXTIME(node.created+$timezone))";
    $this->format = 'F';
  }

  /**
   * Provide a link to the next level of the view
   */
  function summary_link($data, $url) {
    $value = $data->{$this->base_alias};
    $created = $data->{$this->name_alias};
    return l(format_date($created, 'custom', $this->format), "$url/$value");
  }

  /**
   * Provide a link to the next level of the view
   */
  function title($data, $url) {
    return format_date(strtotime("2005" . $this->argument . "15"), 'custom', $this->format, 0);
  }
}

/**
 I Filter by node type
 */
class views_handler_filter_node_type extends views_handler_filter_in_operator {
  function construct() {
    parent::construct();
    $this->value_title = t('Node type');
    $types = node_get_types();
    foreach ($types as $type => $info) {
      $options[$type] = $info->name;
    }
    $this->value_options = $options;
  }
}


/**
 * Implementation of hook_views_plugins
 */
function node_views_plugins() {
  return array(
    'module' => 'views', // This just tells our themes are elsewhere.
    'row' => array(
      'node' => array(
        'title' => t('Node'),
        'help' => t('Display the node with standard node view.'),
        'handler' => 'views_row_plugin_node_view',
        'theme' => 'views_view_row_node',
        'base' => array('node'), // only works with 'node' as base.
        'uses options' => TRUE,
      ),
    ),
  );
}

/**
 * Plugin which performs a node_view on the resulting object.
 *
 * @ingroup views_row_plugins
 */
class views_row_plugin_node_view extends views_row_plugin {
  function options($display) {
    return array(
      'teaser' => TRUE,
    );
  }

  function options_form(&$form, &$form_state) {
    $form['teaser'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display only teaser'),
      '#default_value' => $this->options['teaser'],
    );
  }
  /**
   * Override the behavior of the render() function.
   */
  function render($row) {
    return theme(array('views_view_row_node__' . $this->view->name, 'views_view_row_node'), $this->view, $this->options, $row);
  }
}

/**
 * Template helper for theme_views_view_row_node
 */
function template_preprocess_views_view_row_node(&$vars) {
  $vars['node'] = ''; // make sure var is defined.
  $nid = $vars['row']->nid;
  if (!is_numeric($nid)) {
    return;
  }

  $node = node_load($nid);

  if (empty($node)) {
    return;
  }

  // @todo add settings so that this can choose teaser, links, etc.
  $vars['node'] = node_view($node, TRUE);
}

/**
 * @}
 */
