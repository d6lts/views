<?php
// $Id$
/**
 * @file
 *
 * Provide views data and handlers for upload tables that are not represented by
 * their own module.
 */

/**
 * @defgroup views_upload_module upload.module handlers
 *
 * @{
 */

/**
 * Implementation of hook_views_data()
 */
function upload_views_data() {
  $data = array();

  // ----------------------------------------------------------------------
  // upload table

  $data['upload']['table']['group']  = t('Upload');

  $data['upload']['table']['join'] = array(
    'node' => array(
      'left_field' => 'nid',
      'field' => 'nid',
    ),
    'node_revisions' => array(
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    'users' => array(
      'left_table' => 'node',
      'left_field' => 'nid',
      'field' => 'nid',
    ),
    'files' => array(
      'left_field' => 'fid',
      'field' => 'fid',
    ),
  );

  $data['upload']['fid'] = array(
    'title' => t('Attached files'),
    'help' => t('All files attached to a node with upload.module.'),
    'field' => array(
      'handler' => 'views_handler_field_upload_fid',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_upload_fid',
      'title' => t('Has attached files'),
      'help' => t('Only display items with attached files. This can cause duplicates if there are multiple attached files.'),
    ),
    'relationship' => array(
      'title' => t('Uploaded files'),
      'help' => t('Add a relationship to gain access to more file data for files uploaded by upload.module.'),
      'base' => 'files',
      'field' => 'fid',
      'handler' => 'views_handler_relationship',
      'label' => t('Files'),
    ),
  );

  $data['upload']['description'] = array(
    'title' => t('Description'),
    'help' => t('The description of the uploaded file.'),
    'field' => array(
      'handler' => 'views_handler_field_upload_description', // @todo -- make this linkable, but it's a bit tough
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  $data['upload']['list'] = array(
    'title' => t('Listed'),
    'help' => t('Whether or not the file is marked to be listed.'),
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

  $data['upload']['weight'] = array(
    'title' => t('Weight'),
    'help' => t('The weight, used for sorting.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );

  return $data;
}

/**
 * Field handler to provide a list of roles.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_upload_fid extends views_handler_field_prerender_list {
  function construct() {
    parent::construct();
    $this->additional_fields['nid'] = array('table' => 'node', 'field' => 'nid');
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_to_file'] = array(
      '#title' => t('Link this field to download the file'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['link_to_file']),
    );

    $form['only_listed'] = array(
      '#title' => t('Only show "listed" file attachments'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['only_listed']),
    );
  }


  function query() {
    $this->add_additional_fields();
    $this->field_alias = $this->aliases['nid'];
  }

  function pre_render($results) {
    $nids = array();
    $this->items = array();

    $data = array();
    foreach ($results as $result) {
      $nids[] = $result->{$this->aliases['nid']};
    }

    if ($nids) {
      // Support "only listed files" option.
      $where = '';
      if (!empty($this->options['only_listed'])) {
        $where = " AND u.list <> 0";
      }
      $result = db_query("SELECT u.nid, u.fid, f.filepath, u.description FROM {upload} u LEFT JOIN {files} f ON f.fid = u.fid WHERE u.nid IN (" . implode(', ', $nids) . ")$where ORDER BY u.weight, u.description");
      while ($file = db_fetch_object($result)) {
        $this->items[$file->nid][$file->fid] = $this->render_link(check_plain($file->description), $file);
      }
    }
  }

  /**
   * Render whatever the data is as a link to the file.
   *
   * Data should be made XSS safe prior to calling this function.
   */
  function render_link($data, $values) {
    if (!empty($this->options['link_to_file'])) {
      return l($data, file_create_url($values->filepath), array('html' => TRUE));
    }
    else {
      return $data;
    }
  }
}

/**
 * Field handler to provide a list of roles.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_upload_description extends views_handler_field_prerender_list {
  function init(&$view, &$options) {
    parent::init($view, $options);
    if (!empty($options['link_to_file'])) {
      $this->additional_fields['fid'] = 'fid';
    }
  }

  function options_form(&$form, &$form_state) {
    $form['link_to_file'] = array(
      '#title' => t('Link this field to download the file'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['link_to_file']),
    );
  }

  function pre_render($results) {
    if (empty($this->options['link_to_file'])) {
      return;
    }

    $nids = array();
    $this->items = array();

    $data = array();
    foreach ($results as $result) {
      $fids[] = $result->{$this->aliases['fid']};
    }

    if ($fids) {
      // Support "only listed files" option.
      $result = db_query("SELECT f.fid, f.filepath FROM {files} f WHERE f.fid IN (" . implode(', ', $fids) . ")");
      while ($file = db_fetch_object($result)) {
        $this->items[$file->fid] = $file;
      }
    }
  }

  function render($value) {
    return $this->render_link($value->{$this->field_alias}, $value);

  }

  /**
   * Render whatever the data is as a link to the file.
   *
   * Data should be made XSS safe prior to calling this function.
   */
  function render_link($data, $value) {
    if (!empty($this->options['link_to_file'])) {
      $values = $this->items[$value->{$this->aliases['fid']}];
      return l($data, file_create_url($values->filepath), array('html' => TRUE));
    }
    else {
      return $data;
    }
  }
}

/**
 * Filter by whether or not a node has attached files from the upload module
 *
 * @ingroup views_filter_handlers
 */
class views_handler_filter_upload_fid extends views_handler_filter_boolean_operator {
  function construct() {
    parent::construct();
    $this->value_value = t('Has attached files');
  }

  function query() {
    $this->ensure_my_table();
    $this->query->add_where($this->options['group'], "$this->table_alias.$this->real_field " . (empty($this->value) ? 'IS' : 'IS NOT') . " NULL");
  }
}

/**
 * @}
 */
