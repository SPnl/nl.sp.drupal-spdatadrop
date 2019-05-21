<?php

/**
 * @file
 * Drush functionality.
 */

/**
 * Drush command definition.
 */
function spdatadrop_drush_command() {
  $items = array();

  $items['spdatadrop-retry'] = array(
    'description' => "Retry syncs that gave error.",
    'aliases'     => array('spdr'),
  );

  $items['spdatadrop-sync'] = array(
    'description' => "sync.",
    'aliases'     => array('spds'),
  );

  return $items;
}

/**
 * Drush command implementation.
 */
function drush_spdatadrop_retry() {
  $num_updated = db_update('spdatadrop_syncstate')
    ->fields(array(
      'state' => 0,
    ))
    ->condition('state', 2)
    ->execute();
  watchdog('spdatadrop', 'Reset @updated syncs that gave an error. They will be retried on next cron run.', array('@updated' => $num_updated));
  drupal_set_message('Reset ' . $num_updated . ' syncs that gave an error. They will be retried on next cron run.');
}

/**
 * Syncs not yet synced webform submissions.
 */
function drush_spdatadrop_sync() {
  spdatadrop_customcron();
}
