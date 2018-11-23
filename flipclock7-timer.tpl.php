<?php

/**
 * @file
 * Provides a template for the Countdown Timer Field.
 */
?>

<div class="flipclock7">
  <?php if ($heading): ?>
    <h3><?php print $heading; ?></h3>
  <?php endif; ?>
  <div id="<?php print $timer_id; ?>" class="flipclock7-countdown clock">
  </div>
</div>
