<?php
// $Id$
/**
 * @file views-ui-edit-view.tpl.php
 * Template for the primary view editing window.
 */
?>
<?php if ($locked): ?>
  <div class="view-locked">
    <?php print t('This view is locked by user !user, and may not be modified. This lock is !age old. Click here to !break.', array('!user' => $locked, '!age' => $lock_age, '!break' => $break)); ?>
  </div>
<?php endif; ?>
<div class="views-basic-info<?php if (!empty($view->changed)) { print " changed"; }?>">
  <?php if (!is_numeric($view->vid)): ?>
    <div class="view-changed view-new"><?php print t('New view'); ?></div>
  <?php else: ?>
    <div class="view-changed"><?php print t('Changed view'); ?></div>
  <?php endif; ?>
  <?php print t('View %name, displaying items of type <b>@base</b>.',
      array('%name' => $view->name, '@base' => $base_table)); ?>
</div>

<?php print $tabs; ?>

<div id="views-ajax-form">
  <div id="views-ajax-title">
    <?php // This is initially empty ?>
  </div>
  <div id="views-ajax-pad">
    <?php /* This is sent in because it is also sent out through settings and
    needs to be consistent. */ ?>
    <?php print $message; ?>
  </div>
</div>

<?php print $save_button ?>

<div id='views-live-preview'>
  <?php print $preview ?>
</div>