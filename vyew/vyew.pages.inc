<?php

function vyew_room($uid) {
  $output = "";
  
  $u = user_load($uid);
  if ($u) {
    $name = $u->name;
  } else {
    $u = user_load(array('name' => $uid));
    if ($u) {
      $name = $u->name;
    } else {
      return drupal_not_found();
    }
  }
  $room = vyew_room_settings_load($u->uid);

  //  $vyew = new VyewAPI($room->api_key, $room->api_secret);


  //  $output .= '<iframe src="http://vyew.com/v4latest/content/vyewmain.php?mid=072935_6311881" style="width: 90%; height: 500px;" scrolling="no" marginwidth="0" marginheight="0" frameborder="0" vspace="0" hspace="0"></iframe>';
  $output .= '<iframe src="http://vyew.com/v4latest/content/vyewmain.php?mid=072935_6311881" style="width: 90%; height: 500px;"></iframe>';
  //  $output .= '<iframe src="http://vyew.com/072935/Untitled_Room" style="width: 90%; height: 500px" scrolling="no" marginwidth="0" marginheight="0" frameborder="0" vspace="0" hspace="0"></iframe>';

  return $output;
}