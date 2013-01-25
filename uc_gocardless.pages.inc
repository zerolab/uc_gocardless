<?php

function uc_gocardless_redirect($order_id, $cart_id) {
	$confirm_params = array(
  		'resource_id'    => $_GET['resource_id'],
  		'resource_type'  => $_GET['resource_type'],
  		'resource_uri'   => $_GET['resource_uri'],
  		'signature'      => $_GET['signature']
		);

		// State is optional
		if (isset($_GET['state'])) {
 			 $confirm_params['state'] = $_GET['state'];
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

  drupal_goto('cart/checkout/complete');
}





function uc_gocardless_webhook() {

$webhook = file_get_contents('php://input');
$webhook_array = json_decode($webhook, true);
$webhook_valid = GoCardless::validate_webhook($webhook_array['payload']);

if ($webhook_valid == TRUE) {

  $data = $webhook_array['payload'];

  if ($data['resource_type'] == 'bill' && $data['action'] == 'created') {

    foreach ($data['bill'] as $bill) {

      // Logic to cancel the subscription in your system goes here

      // You can use $subscription['resource_id'] to look up the subscription
      // in the GoCardless API if required

    }

  }

  // Send a success header
  header('HTTP/1.1 200 OK');

} else {

  header('HTTP/1.1 403 Invalid signature');

}



}