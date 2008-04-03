<?php
// $Id$
/**
 * @file
 * Provide views data and handlers for comment.module
 */

/**
 * @defgroup views_comment_module comment.module handlers
 *
 * Includes the tables 'comments' and 'node_comment_statistics'
 * @{
 */

/**
 * Implementation of hook_views_data()
 */
function comment_views_data() {
  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.
  $data['comments']['table']['group']  = t('Comment');

  $data['comments']['table']['base'] = array(
    'field' => 'cid',
    'title' => t('Comments'),
    'help' => t("Your site's comments."),
  );

  //joins
  $data['comments']['table']['join'] = array(
    //...to the node table
    'node' => array(
      'handler' => 'views_join',
      'arguments' => array('comments', 'node', 'nid', 'nid'),
    ),
    //...to the user table
    'users' => array(
      'handler' => 'views_join',
      'arguments' => array('comments', 'users', 'uid', 'uid'),
    ),
  );

  // ----------------------------------------------------------------
  // Fields

  //subject
  //choice to mark as new/updated etc
  $data['comments']['subject'] = array(
    'title' => t('Subject'),
    'help' => t('The subject of the comment.'),
    'field' => array(
      'field' => 'subject',
      'handler' => 'views_handler_field_comment',
      'click sortable' => TRUE,
    ),
  );

  //comment (the comment body)
  $data['comments']['comment'] = array(
    'title' => t('Comment body'),
    'help' => t('The body of the comment.'),
    'field' => array(
      'handler' => 'views_handler_field_markup',
      'format' => 'format',
    ),
  );

  //cid
  $data['comments']['cid'] = array(
    'title' => t('Comment ID'),
    'help' => t('The commment ID of the field'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
  );

  //name (of comment author)
  $data['comments']['name'] = array(
    'title' => t("Author's name"),
    'help' => t('The name of the poster.'),
    'field' => array(
      'handler' => 'views_handler_field_username_comment',
      'click sortable' => TRUE,
    ),
  );

  //homepage
  $data['comments']['homepage'] = array(
    'title' => t("Author's website"),
    'help' => t("The website address of the comment's author. Can be a link. The homepage can also be linked with the Name field."),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
  );

  //timestamp (when comment was posted)
  $data['comments']['timestamp'] = array(
    'title' => t('Created date'),
    'help' => t('Date and time of when the comment was posted.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  //status (approved or not)
  $data['comments']['status'] = array(
    'title' => t('Approval status'),
    'help' => t('The status of the comment. Either approved or not.'),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => 'Approved',
    ),
  );

  //what about options for editing, deleting, replying etc?

  //how to deal with pid (parent id... ie threading)

  // ----------------------------------------------------------------------
  // node_comment_statistics table

  //define the group
  $data['node_comment_statistics']['table']['group']  = t('Node Comment Statistics');

  //joins
  $data['node_comment_statistics']['table']['join'] = array(
    //...to the node table
    'node' => array(
      'handler' => 'views_join',
      'arguments' => array('node_comment_statistics', 'node', 'nid', 'nid'),
     ),
  );

  //last_comment_timestamp
  $data['node_comment_statistics']['last_comment_timestamp'] = array(
    'title' => t('Created time'),
    'help' => t('Date and time of when the last comment was posted.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  //last_comment_name (author's name)
  $data['node_comment_statistics']['last_comment_name'] = array(
    'title' => t("Last author's name"),
    'help' => t('The name of the author of the last posted comment.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
  );

  //comment_count
  $data['node_comment_statistics']['comment_count'] = array(
    'title' => t('Comment count'),
    'help' => t('The number of comments a node has.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
  );

  //todo: last_comment_timestamp (#filter by when the last comment or node change was made)
  //todo: last_changed (notafield). Show the time when the last comment or node change was made

  return $data;
}

/**
 * Field handler to allow linking to a comment
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_comment extends views_handler_field {
  /**
   * Override init function to provide generic option to link to comment.
   */
  function init(&$view, &$data) {
    parent::init($view, $data);
    if (isset($data->link_to_comment)) {
      $this->additional_fields['cid'] = 'cid';
      $this->additional_fields['nid'] = 'nid';
    }
  }

  function options(&$options) {
    parent::options($options);
    $options['link_to_comment'] = TRUE;
  }

  /**
   * Provide link-to-comment option
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_to_comment'] = array(
      '#title' => t('Link this field to its comment'),
      '#type' => 'checkbox',
      '#default_value' => $this->options['link_to_comment'],
    );
  }

  function render_link($data, $values) {
    if (!empty($this->options['link_to_comment'])) {
      return l($data, "node/". $values->{$this->aliases['nid']}, array('html' => TRUE, 'fragment' => "comment-". $values->{$this->aliases['cid']}));
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
 * Field handler to allow linking to a user account or homepage
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_username_comment extends views_handler_field {
  /**
   * Override init function to add uid and homepage fields.
   */
  function init(&$view, &$data) {
    parent::init($view, $data);
    $this->additional_fields['uid'] = 'uid';
    $this->additional_fields['uid'] = 'homepage';
  }

  function options(&$options) {
    parent::options($options);
    $options['link_to_user'] = TRUE;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_to_user'] = array(
      '#title' => t("Link this field to its user or an author's homepage"),
      '#type' => 'checkbox',
      '#default_value' => $this->options['link_to_user'],
    );
  }

  function render_link($data, $values) {
    $account->uid = $values->{$this->aliases['uid']};
    $account->name = $values->{$this->field_alias};
    $account->homepage = $values->{$this->aliases['homepage']};

    if (!empty($this->options['link_to_user'])) {
      return theme('username', $account);
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
 * @}
 */
