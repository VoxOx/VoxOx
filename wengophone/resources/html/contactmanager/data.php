<?php

function subval_sort($a, $subkey) {
	foreach($a as $k=>$v) {
		$b[$k] = strtolower($v[$subkey]);
	}
	asort($b);
	foreach($b as $key=>$val) {
		$c[] = $a[$key];
	}
	return $c;
}

switch($_REQUEST['data']) {
	case 'groups':
		$results = array(
						array(id => 1, groupname => 'Friends', totalingroup => 12),
						array(id => 2, groupname => 'Family', totalingroup => 5),
						array(id => 3, groupname => 'Co-Workers', totalingroup => 7),
						array(id => 4, groupname => 'VoxOx', totalingroup => 0)
					);
		break;
	case 'contacts':
		if($_REQUEST['group']=='Friends') {
			$results = array(	
								array(id => 23, fname => '', lname => '', company => '24 Hour Fitness', group => 'Friends', iscompany => true),
								array(id => 24, fname => '', lname => '', company => '24 Hour Fitness 2', group => 'Friends', iscompany => true),
								array(id => 2, fname => 'Bryan', lname => 'Hertz', company => '', group => 'Friends', iscompany => false),
								array(id => 4, fname => 'Kevin', lname => 'Hertz', company => '', group => 'Friends', iscompany => false),
								array(id => 7, fname => 'Corey', lname => 'Hertz', company => '', group => 'Friends', iscompany => false),
								array(id => 10, fname => 'Felipe', lname => 'Lopez', company => '', group => 'Friends', iscompany => false),
								array(id => 12, fname => 'Jorge', lname => 'Guntanis', company => '', group => 'Friends', iscompany => false),
								array(id => 14, fname => 'Kayla', lname => 'Tran', company => '', group => 'Friends', iscompany => false),
								array(id => 16, fname => 'Michael', lname => 'Faught', company => '', group => 'Friends', iscompany => false),
								array(id => 18, fname => 'Steve', lname => 'Chrapchynski', company => '', group => 'Friends', iscompany => false),
								array(id => 19, fname => 'Tad', lname => 'Nikolich', company => '', group => 'Friends', iscompany => false),
								array(id => 20, fname => 'Bryan', lname => 'Mathews', company => '', group => 'Friends', iscompany => false)
							);
		} else if($_REQUEST['group']=='Family') {
			$results = array(
								array(id => 1, fname => 'Robert', lname => 'Abad', company => '', group => 'Family', iscompany => false),
								array(id => 3, fname => 'Robo', lname => 'Hertz', company => '', group => 'Family', iscompany => false),
								array(id => 4, fname => 'Kevin', lname => 'Hertz', company => '', group => 'Friends', iscompany => false),
								array(id => 8, fname => 'Curtis', lname => 'Kaffer', company => '', group => 'Family', iscompany => false),
								array(id => 9, fname => 'Ed', lname => 'Mallory', company => '', group => 'Family', iscompany => false)
							);
		} else if($_REQUEST['group']=='Co-Workers') {
			$results = array(
								array(id => 5, fname => 'Connie', lname => 'Sanders', company => '', group => 'Co-Workers', iscompany => false),
								array(id => 6, fname => 'Chris', lname => 'Florio', company => '', group => 'Co-Workers', iscompany => false),
								array(id => 11, fname => 'Jeff', lname => 'Theinert', company => '', group => 'Co-Workers', iscompany => false),
								array(id => 13, fname => 'Jose', lname => 'Baltodano', company => '', group => 'Co-Workers', iscompany => false),
								array(id => 15, fname => 'Lisa', lname => 'Kendall', company => '', group => 'Co-Workers', iscompany => false),
								array(id => 17, fname => 'Sharon', lname => 'Holmes', company => '', group => 'Co-Workers', iscompany => false),
								array(id => 21, fname => 'Daniel', lname => 'Corbe', company => '', group => 'Co-Workers', iscompany => false),
								array(id => 22, fname => 'Barrent', lname => 'Brown', company => '', group => 'Co-Workers', iscompany => false)
							);
		} else {
			$results = array(
								array(id => 1, fname => 'Robert', lname => 'Abad', company => '', group => 'Family', iscompany => false),
								array(id => 2, fname => 'Bryan', lname => 'Hertz', company => '', group => 'Friends', iscompany => false),
								array(id => 3, fname => 'Robo', lname => 'Hertz', company => '', group => 'Family', iscompany => false),
								array(id => 4, fname => 'Kevin', lname => 'Hertz', company => '', group => 'Friends', iscompany => false),
								array(id => 5, fname => 'Connie', lname => 'Sanders', company => '', group => 'Co-Workers', iscompany => false),
								array(id => 6, fname => 'Chris', lname => 'Florio', company => '', group => 'Co-Workers', iscompany => false),
								array(id => 7, fname => 'Corey', lname => 'Hertz', company => '', group => 'Friends', iscompany => false),
								array(id => 8, fname => 'Curtis', lname => 'Kaffer', company => '', group => 'Family', iscompany => false),
								array(id => 9, fname => 'Ed', lname => 'Mallory', company => '', group => 'Family', iscompany => false),
								array(id => 10, fname => 'Felipe', lname => 'Lopez', company => '', group => 'Friends', iscompany => false),
								array(id => 11, fname => 'Jeff', lname => 'Theinert', company => '', group => 'Co-Workers', iscompany => false),
								array(id => 12, fname => 'Jorge', lname => 'Guntanis', company => '', group => 'Friends', iscompany => false),
								array(id => 13, fname => 'Jose', lname => 'Baltodano', company => '', group => 'Co-Workers', iscompany => false),
								array(id => 14, fname => 'Kayla', lname => 'Tran', company => '', group => 'Friends', iscompany => false),
								array(id => 15, fname => 'Lisa', lname => 'Kendall', company => '', group => 'Co-Workers', iscompany => false),
								array(id => 16, fname => 'Michael', lname => 'Faught', company => '', group => 'Friends', iscompany => false),
								array(id => 17, fname => 'Sharon', lname => 'Holmes', company => '', group => 'Co-Workers', iscompany => false),
								array(id => 18, fname => 'Steve', lname => 'Chrapchynski', company => '', group => 'Friends', iscompany => false),
								array(id => 19, fname => 'Tad', lname => 'Nikolich', company => '', group => 'Friends', iscompany => false),
								array(id => 20, fname => 'Bryan', lname => 'Mathews', company => '', group => 'Friends', iscompany => false),
								array(id => 21, fname => 'Daniel', lname => 'Corbe', company => '', group => 'Co-Workers', iscompany => false),
								array(id => 22, fname => 'Barrent', lname => 'Brown', company => '', group => 'Co-Workers', iscompany => false),
								array(id => 23, fname => '', lname => '', company => '24 Hour Fitness', group => 'Friends', iscompany => true),
								array(id => 24, fname => '', lname => '', company => '24 Hour Fitness 2', group => 'Friends', iscompany => true)
							);
		}
		
		if($_REQUEST['sortby']=='fname') {
			$sortyby = 'fname';
		} else {
			$sortyby = 'lname';
		}
		
		$results = subval_sort($results,$sortyby); 
		break;
	case 'contact_1':
		$results = array(
						pinfo => array(id => 1, fname => 'John', lname => 'Smith', company => '24 Hour Fitness', title => 'Sales Agent', iscompany => true),
						group => array(id => 1, groupname => 'testgroup'),
						phones => array(
										array(id => 1, label => 'Home', phone => '858-222-4444'),
										array(id => 2, label => 'Mobile', phone => '619-111-5555')
									  ),
						emails => array(
										array(id => 1, label => 'Work', email => 'email1@voxox.com'),
										array(id => 2, label => 'Home', email => 'email2@voxox.com'),
										array(id => 3, label => 'Other', email => 'email3@voxox.com')
						),
						services_im => array(
										array(id => 1, label => 'VoxOx - jsmith', service => 'screenname1'),
										array(id => 2, label => 'AIM - jsmith', service => 'screenname2'),
										array(id => 3, label => 'ICQ - jsmith', service => 'screenname3'),
										array(id => 4, label => 'Skype - jsmith', service => 'screenname4')
										 ),
						services_social => array(
										array(id => 1, label => 'Facebook', service => 'screenname1'),
										array(id => 2, label => 'Myspace', service => 'screenname2'),
										array(id => 3, label => 'Twitter', service => 'screenname3')
										 ),
						addresses => array()
						);
		break;
	case 'contact_2':
		$results = array(
						pinfo => array(id => 1, fname => 'Jane', lname => 'Smith', company => '24 Hour Fitness', title => 'Sales Agent', iscompany => false),
						group => array(id => 1, groupname => 'testgroup'),
						phones => array(
										array(id => 1, label => 'Home', phone => '858-222-4444'),
										array(id => 2, label => 'Mobile', phone => '619-111-5555')
									  ),
						emails => array(
										array(id => 1, label => 'Work', email => 'email1@voxox.com'),
										array(id => 2, label => 'Home', email => 'email2@voxox.com'),
										array(id => 3, label => 'Other', email => 'email3@voxox.com')
						),
						services_im => array(
										array(id => 1, label => 'VoxOx - jsmith', service => 'screenname1'),
										array(id => 2, label => 'AIM - jsmith', service => 'screenname2'),
										array(id => 3, label => 'ICQ - jsmith', service => 'screenname3'),
										array(id => 4, label => 'Skype - jsmith', service => 'screenname4')
										 ),
						services_social => array(
										array(id => 1, label => 'Facebook', service => 'screenname1'),
										array(id => 2, label => 'Myspace', service => 'screenname2'),
										array(id => 3, label => 'Twitter', service => 'screenname3')
										 ),
						addresses => array()
						);
		break;
	case 'labels_groups':
		$results = array(
						array(id => 1, label => 'Friends'),
						array(id => 2, label => 'Family'),
						array(id => 3, label => 'Co-Workers')
						);
		break;
	case 'labels_phone':
		$results = array(
						array(id => 1, label => 'Home'),
						array(id => 2, label => 'Work'),
						array(id => 3, label => 'Mobile'),
						array(id => 4, label => 'Main'),
						array(id => 5, label => 'Fax - Home'),
						array(id => 6, label => 'Fax - Work'),
						array(id => 7, label => 'Pager'),
						array(id => 8, label => 'Other')
						);
		break;
	case 'labels_email':
		$results = array(
						array(id => 1, label => 'Home'),
						array(id => 2, label => 'Work'),
						array(id => 3, label => 'Other')
						);
		break;
	case 'labels_services_im':
		$results = array(
						array(id => 1, label => 'VoxOx - jsmith'),
						array(id => 2, label => 'AIM - jsmith'),
						array(id => 3, label => 'Skype - jsmith'),
						array(id => 4, label => 'AIM - jsmith'),
						array(id => 5, label => 'Yahoo - jsmith')
						);
		break;
	case 'labels_services_social':
		$results = array(
						array(id => 1, label => 'Facebook'),
						array(id => 2, label => 'Myspace'),
						array(id => 3, label => 'Twitter')
						);
		break;
	case 'labels_addresses':
		$results = array(
						array(id => 1, label => 'Home'),
						array(id => 2, label => 'Work'),
						array(id => 3, label => 'Other')
						);
		break;
	case 'newgroup':
		// need to check that the group doesn't exist already before adding
		$groups=array(1=>'Friends', 2=>'Family', 3=>'Co-Workers');

		$key = array_search($_REQUEST['newgroupname'],$groups);
		
		if($key != '') {
			$results = array(response => true, id => 100);
		} else {
			$results = array(response => false, id => '');
		}
		break;
}	

echo json_encode($results);

//echo "<pre>";
//print_r($results);
?>