<?php

function vyew_room($uid, $appointment_event_id = "") {
  $output = "";
  
  $u = eto_user_load($uid);
  if ($u) {
    $name = $u->name;
  } else {
    $u = eto_user_load(array('name' => $uid));
    if ($u) {
      $name = $u->name;
    } else {
      drupal_not_found();
      exit;
    }
  }

  // we can only view this if we own it, we have a relationship, we're an admin, OR it's the sample
  global $user;
  if ( ($u->name == "sample") 
       || ($user->uid == $u->uid)
       || (user_access("administer the site"))
       || is_assigned_to($u, $user) ) {
    
    // if we have an appointment, display skype stuff up top
    if ($appointment_event_id) {
      $event = sb_appointment_event_load($appointment_event_id);
      $appointment = sb_appointment_load($event->appointment_id);
      $tutor_string = _construct_participant_string($appointment, "tutor");
      $student_string = _construct_participant_string($appointment, "student");
      $output .= theme('meeting_participants', $tutor_string, $student_string);
    }
    global $user;
    
    if ($user->uid == $appointment->tutor_uid) {
      // give us a session log link for the end
      $session_log = "<p>" 
	. l("When this session is over, click here to record your session log.", 
	    appointment_url("", $event->appointment_id, $event->appointment_event_id, "log/edit"))
	. "</p>";
    }
    
    $room = vyew_room_settings_load($u->uid);

    if (strlen($room->meeting_id) > 0) {
      // going to want to use user settings to set how this appears: fixed vs fluid, and height
      $output .= $session_log;
      $output .= '<iframe src="http://vyew.com/embed.php?mid=' . $room->meeting_id . '" style="width: 95%; height: 700px;"></iframe>';
      $output .= $session_log;
    } else {
      $output .= "<h2>Sorry, there is no room set up yet!</h2>";
      if (user_access('edit any vyew room settings')) {
	$output .= "<p>" . l("Click here to add a Vyew room for this user account.", "users/" . $u->name . "/room/edit") . "</p>\n";
      } else {
	$output .= "<p>Please " . l("contact us", "contact/") . " if you think you should have a room set up and you don't.</p>\n";
      }
    }

    return $output;

  } else {

    drupal_access_denied();
    exit;
  }
}