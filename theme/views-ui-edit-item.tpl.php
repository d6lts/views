<?php
// $Id$
/**
 * @file views-ui-edit-item.tpl.php
 *
 * This template handles the printing of fields/filters/sort criteria/arguments or relationships.
 */
?>
<?php print $rearrange; ?>
<?php print $add; ?>
<dt<?php if ($overridden) { print ' class="overridden"'; }?>>
  <?php print $title; ?>
</dt>  

<?php if (empty($fields)): ?>
  <dd><?php print t('None defined'); ?></dd>
<?php else: ?>
  <?php foreach ($fields as $field): ?>
    <?php if (!empty($fields['links'])): ?>
      <?php print $field['links']; ?>
    <?php endif; ?>
    <dd class="<?php print $field['class']; ?>">
      <?php print $field['title']; ?>
      <?php print $field['info']; ?>
    </dd>
  <?php endforeach; ?>
<?php endif; ?>
