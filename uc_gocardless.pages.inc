<?php

/**
 * @file
 *   uc_gocardless.pages.inc
 */

/**
 *  Redirect callback
 */
function uc_gocardless_complete($order) {
  uc_gocardless_initialize_library();

  $confirm_params = array(
    'resource_id'    => check_plain($_GET['resource_id']),
    'resource_type'  => check_plain($_GET['resource_type']),
    'resource_uri'   => check_plain($_GET['resource_uri']),
    'signature'      => check_plain($_GET['signature'])
  );

  // State is optional
  if (isset($_GET['state'])) {
    $confirm_params['state'] = check_plain($_GET['state']);
  }

  $confirmed_resource = GoCardless::confirm_resource($confirm_params);

  if (intval($_SESSION['cart_order']) != $order->order_id) {
    drupal_set_message(t('Thank you for your order! GoCardless will notify us once your payment has been processed.'));
    drupal_goto('cart');
  }

  // Ensure the payment method is GoCardless.
  if ($order->payment_method != 'uc_gocardless') {
    drupal_goto('cart');
  }

  // This lets us know it's a legitimate access of the complete page.
  $_SESSION['do_complete'] = TRUE;

  // Save the GoCardless record id <-> order id link
  $record = new StdClass;
  $record->order_id = $order->order_id;
  $record->resource_id = $confirmed_resource->id;

  drupal_write_record('uc_order_gocardless', $record);

  drupal_goto('cart/checkout/complete');
}

/**
 * Webhook callback
 */
function uc_gocardless_webhook() {
  uc_gocardless_initialize_library();

  $webhook = file_get_contents('php://input');
  $webhook_array = json_decode($webhook, true);
  $webhook_valid = GoCardless::validate_webhook($webhook_array['payload']);

  if (1 || $webhook_valid == TRUE && $webhook_array['payload']['resource_type'] == 'bill') {
    foreach ($webhook_array['payload']['bills'] as $bill) {
      // Get the order id based on bill resource id
      $order_id = db_result(db_query("SELECT order_id FROM {uc_order_gocardless} WHERE resource_id = '%s' ", $bill['id']));
      if (!$order_id || !($order = uc_order_load($order_id)) ) {
        watchdog('uc_gocardless', t('Could not find the corresponding order for GoCardless id: @id', array('@id' => $bill['id'])));
        continue;
      }

      switch ($webhook_array['payload']['action']) {
        case 'paid':
          $context = array(
            'revision' => 'formatted-original',
            'type' => 'amount',
          );
          $options = array(
            'sign' => FALSE,
          );

          $comment = t('GoCardless resource ID: @id', array('@id' => $bill['id']));

          uc_payment_enter($order_id, 'gocardless', $bill['amount'], $order->uid, NULL, $comment);
          uc_cart_complete_sale($order);
          uc_order_update_status($order_id, 'completed');
          uc_order_comment_save($order_id, 0, t('GoCardless reported a payment of @amount @currency.', array('@amount' => uc_price($bill['amount'], $context, $options), '@currency' => $order->currency)));
          break;
        case 'failed':
          $message = t('GoCardless bill could not be debited from the customer account');
          uc_order_comment_save($order_id, 0, $message, 'order', 'canceled');
          uc_order_update_status($order_id, 'canceled');
          break;
        case 'refunded':
          uc_order_comment_save($order_id, 0, t('Order refunded via GoCardless.'), 'order', 'canceled');
          uc_order_update_status($order_id, 'canceled');
          break;
      }
    }

    // Send a success header
    header('HTTP/1.1 200 OK');
  }
  else {
    header('HTTP/1.1 403 Invalid signature');
  }
}
