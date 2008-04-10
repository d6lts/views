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
    'title' => t('Comment'),
    'help' => t("Comments are responses to node content."),
  );

  //joins
  $data['comments']['table']['join'] = array(
    //...to the node table
    'node' => array(
      'left_field' => 'nid',
      'field' => 'nid',
    ),
    //...to the user table
    'users' => array(
      'left_field' => 'uid',
      'field' => 'uid',
    ),
  );

  // ----------------------------------------------------------------
  // Fields

  //subject
  //@todo choice to mark as new/updated etc
  $data['comments']['subject'] = array(
    'title' => t('Subject'),
    'help' => t('The subject of the comment.'),
    'field' => array(
      'field' => 'subject',
      'handler' => 'views_handler_field_comment',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
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
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
  );

  //cid
  $data['comments']['cid'] = array(
    'title' => t('ID'),
    'help' => t('The commment ID of the field'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument',
    ),
  );

  //name (of comment author)
  $data['comments']['name'] = array(
    'title' => t('Author'),
    'help' => t('The name of the poster.'),
    'field' => array(
      'handler' => 'views_handler_field_username_comment',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
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
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
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
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );

  //status (approved or not)
  $data['comments']['status'] = array(
    'title' => t('Approval status'),
    'help' => t('The status of the comment. Either approved or not.'),
    'field' => array(
      // @todo: this needs reversing
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_comment_status',
      'label' => t('Approved'),
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    //@todo: make this argument work well for status
    'argument' => array(
      'handler' => 'views_handler_argument',
    ),
  );

  //link to view comment
  $data['comments']['view_comment'] = array(
    'field' => array(
      'title' => t('View link'),
      'help' => t('Provide a simple link to view the comment.'),
      'handler' => 'views_handler_field_comment_link',
    ),
  );

  //link to edit comment
  $data['comments']['edit_comment'] = array(
    'field' => array(
      'title' => t('Edit link'),
      'help' => t('Provide a simple link to edit the comment.'),
      'handler' => 'views_handler_field_comment_link_edit',
    ),
  );

  //link to delete comment
  $data['comments']['delete_comment'] = array(
    'field' => array(
      'title' => t('Delete link'),
      'help' => t('Provide a simple link to delete the comment.'),
      'handler' => 'views_handler_field_comment_link_delete',
    ),
  );

  //link to reply to comment
  $data['comments']['replyto_comment'] = array(
    'field' => array(
      'title' => t('Reply-to link'),
      'help' => t('Provide a simple link to reply to the comment.'),
      'handler' => 'views_handler_field_comment_link_reply',
    ),
  );

  $data['comments']['thread'] = array(
    'field' => array(
      'title' => t('Depth'),
      'help' => t('Display the depth of the comment if it is threaded'),
      'handler' => 'views_handler_field_comment_depth',
    ),
    'sort' => array(
      'title' => t('Thread'),
      'help' => t('Sort by the threaded order. This will keep child comments together with their parents.'),
      'handler' => 'views_handler_sort_comment_thread',
    ),
  );

  $data['comments']['pid'] = array(
    'field' => array(
      'title' => t('Parent CID'),
      'help' => t('Display the CID of the parent comment'),
      'handler' => 'views_handler_field',
    ),
  );

  // ----------------------------------------------------------------------
  // node_comment_statistics table

  //define the group
  $data['node_comment_statistics']['table']['group']  = t('Node Comment Statistics');

  //joins
  $data['node_comment_statistics']['table']['join'] = array(
    //...to the node table
    'node' => array(
      'left_field' => 'nid',
      'left' => 'nid',
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
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );

  //last_comment_name (author's name)
  $data['node_comment_statistics']['last_comment_name'] = array(
    'title' => t("Last author's name"),
    'help' => t('The name of the author of the last posted comment.'),
    'field' => array(
       //@todo as a link to author?
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  //comment_count
  $data['node_comment_statistics']['comment_count'] = array(
    'title' => t('Comment count'),
    'help' => t('The number of comments a node has.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument',
    ),
  );

  //todo: number of new comments?

  //todo: last_comment_timestamp (#filter by when the last comment or node change was made)
  //todo: last_changed (notafield). Show the time when the last comment or node change was made

  return $data;
}

/**
 * Sort handler for ordering by thread
 */
class views_handler_sort_comment_thread extends views_handler_sort {
  function query() {
    $this->ensure_my_table();

    //Read comment_render() in comment.module for an explanation of the
    //thinking behind this sort.
    if ($this->options['order'] == 'DESC') {
      $this->query->add_orderby($this->table_alias, $this->real_field, $this->options['order']);
    }
    else {
      $alias = $this->table_alias .'_'. $this->real_field .'asc';
      //@todo is this secure?
      $this->query->add_orderby(NULL, "SUBSTRING({$this->table_alias}.{$this->real_field}, 1, (LENGTH({$this->table_alias}.{$this->real_field}) - 1))", $this->options['order'], $alias);
    }
  }
}

/**
 * Field handler to display the depth of a comment
 */
class views_handler_field_comment_depth extends views_handler_field {
  /**
   * Work out the depth of this comment
   */
  function render($values) {
    return count(explode('.', $values->comments_thread)) - 1;
  }
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
    if (!empty($data->link_to_comment)) {
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
    $this->additional_fields['homepage'] = 'homepage';
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
 * Filter handler on comment published status
 */
class views_handler_filter_comment_status extends views_handler_filter_boolean_operator {
  function query() {
    $this->ensure_my_table();
    $this->query->add_where($this->options['group'], "$this->table_alias.$this->real_field " . (empty($this->value) ? '<>' : '=') . " %d", COMMENT_PUBLISHED);
  }
}

/**
 * Base field handler to present a link.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_comment_link extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['cid'] = 'cid';
    $this->additional_fields['nid'] = 'nid';
  }

  function options(&$options) {
    parent::options($options);
    $options['text'] = '';
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['text'] = array(
      '#type' => 'textfield',
      '#title' => t('Text to display'),
      '#default_value' => $this->options['text'],
    );
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
 }

  function render($values) {
    $text = !empty($this->options['text']) ? $this->options['text'] : t('view');
    return l($text, "node/". $values->{$this->aliases['nid']}, array('html' => TRUE, 'fragment' => "comment-". $values->{$this->aliases['cid']}));
  }
}

/**
 * Field handler to present a link node edit.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_comment_link_edit extends views_handler_field_comment_link {
  function construct() {
    parent::construct();
    $this->additional_fields['uid'] = 'uid';
  }

  function render($values) {
    // ensure user has access to edit this comment.
    $comment = new stdClass();
    $comment->cid = $values->{$this->aliases['cid']};
    $comment->uid = $values->{$this->aliases['uid']};
    if (!comment_access('edit', $comment)) {
      return;
    }

    $text = !empty($this->options['text']) ? $this->options['text'] : t('edit');
    return l($text, "comment/edit/". $values->{$this->aliases['cid']});
  }
}

/**
 * Field handler to present a link to delete a node.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_comment_link_delete extends views_handler_field_comment_link {
  function render($values) {
    //needs permission to administer comments in general
    if (!user_access('administer comments')) {
      return;
    }
    $text = !empty($this->options['text']) ? $this->options['text'] : t('delete');
    return l($text, "comment/delete/". $values->{$this->aliases['cid']}, array('query' => drupal_get_destination()));
  }
}

/**
 * Field handler to present a link to delete a node.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_comment_link_reply extends views_handler_field_comment_link {
  function render($values) {
    //check for permission to reply to comments
    if (!user_access('post comments')) {
      return;
    }
    $text = !empty($this->options['text']) ? $this->options['text'] : t('reply');
    return l($text, "comment/reply/". $values->{$this->aliases['nid']} .'/'. $values->{$this->aliases['cid']});
  }
}

/**
 * @}
 */
