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
        'operator' => array('=' => 'Exists'),
        'list' => 'views_handler_operator_yesno',
        'list-type' => 'select',
        'handler' => 'views_handler_file_filter_fid_exist',
        'help' => t('Filter weather the node has files for download'),
      ),
      'list' => array(
        'name' => t('File: Listed in file downloads'),
        'operator' => array('=' => 'Equals'),
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
      'filename' => array(
        'name' => t('File: Name'),
        'handler' => array(
          'views_handler_file_filename' => t('Plain'),
          'views_handler_file_filename_download' => t('With download link'),
        ),
        'sortable' => true,
        'addlfields' => array('filepath'),
        'help' => t('Display file name'),
      ),
      'filepath' => array(
        'name' => t('File: Path'),
        'sortable' => false,
        'help' => t('Display Path to File.'),
      ),
    ),
    'sorts' => array(
      'filename' => array(
        'name' => t('File: Sort by Filename'),
        'help' => t('Sort by file name'),
      ),
    ),
  );
  return $tables;
}

function views_handler_file_filename($fieldinfo, $fielddata, $value, $data) {
  return $value;
}

function views_handler_file_filename_download($fieldinfo, $fielddata, $value, $data) {
  return $value ? l($value, check_url(file_create_url($data->files_filepath))) : '';
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

