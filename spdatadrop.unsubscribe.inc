<?php

/**
 * @file
 * Unsubscribe form.
 */

function spdatadrop_unsubscribe_form($form, &$form_state, $encoded_data) {
  $item = array();

  $items['intro_text'] = array(
    '#markup' => '<p>TEST: ' . $encoded_data . '</p>',
  );
  return $items;
}
