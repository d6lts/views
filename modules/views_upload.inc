<?php

// $Id$
/**
 * This include file implements views functionality for the file.inc and
 * upload module
 */

function upload_views_tables() {
  $tables['file_revisions'] = array(
    'name' => 'file_revisions',
    'join' => array(
      'left' => array(
        'table' => 'node',
        'field' => 'vid',
      ),
      'right' => array(
        'field' => 'vid',
      ),
    ),
    'fields' => array(
      'fid' => array(
        'name' => t('File: Id'),
        'sortable' => true,
        'help' => t('File Id which represents the file.'),
      ),
    ),
    'sorts' => array(
      'fid' => array(
        'name' => t('Sort by File Id'),
      ),
    ),
    'filters' => array(
      'fid' => array(
        'name' => t('File: Has file downloads'),
        'operator' => array('=' => t('Exists')),
        'list' => 'views_handler_operator_yesno',
        'list-type' => 'select',
        'handler' => 'views_handler_file_filter_fid_exist',
        'help' => t('Filter weather the node has files for download'),
      ),
      'list' => array(
        'name' => t('File: Listed in file downloads'),
        'operator' => array('=' => t('Equals')),
        'list' => 'views_handler_operator_yesno',
        'list-type' => 'select',
        'help' => t('Filter weather the file is listed in downloads'),
      ),
    ),
  );
  $tables['files'] = array(
    'name' => 'files',
    'join' => array(
      'left' => array(
        'table' => 'file_revisions',
        'field' => 'fid',
      ),
      'right' => array(
        'field' => 'fid'
      ),
    ),
    'fields' => array(
      'all_files' => array(
        'name' => t('File: All files'),
        'notafield' => true,
        'query_handler' => 'views_query_handler_file_all_files',
        'handler' => array(
          'views_handler_file_all_files' => t('All files'),
          'views_handler_file_listed_files' => t('Listed files')),
        'option' => array(
          '#type' => 'select',
          '#options' => array(
            'link' => t('With links'), 
            'nolink' => t('Without links'),
           )),
        'sortable' => false,
        'help' => t('Display all attached files in one field.'),
      ),
      'filename' => array(
        'name' => t('File: Name'),
        'handler' => array(
          'views_handler_file_filename' => t('Plain'),
          'views_handler_file_filename_download' => t('With download link'),
        ),
        'sortable' => true,
        'addlfields' => array('filepath'),
        'option' => 'string',
        'help' => t('Display file name'),
      ),
      'filepath' => array(
        'name' => t('File: Path'),
        'sortable' => false,
        'help' => t('Display Path to File.'),
      ),
      'filesize' => array(
        'name' => t('File: Size'),
        'handler' => 'views_handler_file_size',
        'sortable' => true,
        'help' => t('Display the size of the associated file.'),
      ),
      'filemime' => array(
        'name' => t('File: Mime type'),
        'sortable' => true,
        'help' => t('This filter allows nodes to be filtered by mime type.'),
      ),
    ),
    'filters' => array(
      'filename' => array(
        'name' => t('File: Filename'),
        'operator' => 'views_handler_operator_like',
        'handler' => 'views_handler_filter_like',
        'help' => t('This filter allows nodes to be filtered by the name of attached files.'),
      ),
      'filesize' => array(
        'name' => t('File: Size'),
        'operator' => 'views_handler_operator_gtlt',
        'help' => t('This filter allows nodes to be filtered by file size.'),
      ),
      'filemime' => array(
        'name' => t('File: Mime type'),
        'operator' => 'views_handler_operator_like',
        'handler' => 'views_handler_filter_like',
        'help' => t('This filter allows nodes to be filtered by mime type.'),
      ),
    ),
    'sorts' => array(
      'filename' => array(
        'name' => t('File: Sort by Filename'),
        'help' => t('Sort by file name'),
      ),
      'filesize' => array(
        'name' => t('File: File size'),
        'help' => t('Sort by file size.'),
      ),
      'filemime' => array(
        'name' => t('File: Mime type'),
        'help' => t('Sort by mime type.'),
      ),
    ),
  );
  return $tables;
}

function views_handler_file_filename($fieldinfo, $fielddata, $value, $data) {
  return $value;
}

function views_handler_file_size($fieldinfo, $fielddata, $value, $data) {
  return format_size($value);
}

function views_handler_file_filename_download($fieldinfo, $fielddata, $value, $data) {
  $link_text = $fielddata['options'] ? $fielddata['options'] : $value;
  return $value ? l($link_text, check_url(file_create_url($data->files_filepath))) : '';
}

function views_handler_file_filter_fid_exist($op, $filter, $filterdata, &$query) {
  switch ($op) {
    case 'handler':
      if ($filter['value']) {
        $query->add_where('file_revisions.fid');
      }
      else {
        $query->add_where('ISNULL(file_revisions.fid)');
      }
  }
}

function views_query_handler_file_all_files($field, $fieldinfo, &$query) {
  $query->add_field('vid', 'node');
}

/**
 * Display all files attached to a given node.
 */
function views_handler_file_all_files($fieldinfo, $fielddata, $value, $data, $listed = false) {
  if ($listed) {
    $and = " AND list = 1";
  }
  $links = array();

  $result = db_query("SELECT f.* FROM {file_revisions} fr INNER JOIN {files} f ON f.fid = fr.fid WHERE fr.vid = %d $and", $data->vid);
    while ($file = db_fetch_object($result)) {
    if ($fielddata['options'] == 'nolink') {
      $links[] = check_plain($file->filename);
    }
    else {
      $links[] = l($file->filename, check_url(file_create_url($file->filepath)));
    }
  }
  return implode(' | ', $links);
}

function views_handler_file_listed_files($fieldinfo, $fielddata, $value, $data) {
  return views_handler_file_all_files($fieldinfo, $fielddata, $value, $data, TRUE);
}
