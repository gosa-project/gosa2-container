<?php

require_once("class_ldap.inc");

create_base_and_admin($argv[1], $argv[2], $argv[3], $argv[4] === 1, $argv[5] ?? 'gsa', $argv[6] ?? 'tester');

function create_base_and_admin(string $connection_string, string $admin_dn, string $admin_pw, bool $tls = false, string $gsa_uid = "gsa", string $gsa_pw = 'tester')
{
    /**
     * Build LDAP connection
     * 
     * use gosa.conf style parameters e.g.
     * 
     * <location name="mygosalocation"
     *   ...
     *   <referral URI="ldap://ldap.gosa.example.de/dc=example,dc=de"
     *             adminDn="cn=manager,dc=uni-bonn,dc=de"
     *             adminPassword="manager" />
     * </location>
     */
    $ds = ldap_connect($connection_string);
    if (!$ds) {
        die("Can't bind to LDAP. No check possible!");
    }

    ldap_set_option($ds, LDAP_OPT_PROTOCOL_VERSION, 3);

    if ($tls) {
        ldap_start_tls($ds);
    }

    ldap_bind($ds, $admin_dn, $admin_pw);

    /* Get base to look for naming contexts */
    $sr  = ldap_read($ds, "", "objectClass=*", array("+"));
    $attr = ldap_get_entries($ds, $sr);

    $base = $attr[0]['namingcontexts'][0];

    ldap_close($ds);

    $ldap = new LDAP($admin_dn, $admin_pw, $connection_string, false, $tls);
    $ldap->getSearchResource();

    // Add root object
    $ldap->cd($base);
    $ldap->create_missing_trees(0, $base);
    
    // add gsa
    $uid = $gsa_uid;
    $pw = $gsa_pw;

    $people_ou = "ou=people,"; // Thats the property default.
    $dn = "cn=System Administrator-" . $uid . "," . $people_ou . $base;

    $hash = "{MD5}".base64_encode( pack('H*', md5($pw)));

    $new_user = array();
    $new_user['objectClass'] = array("top", "person", "gosaAccount", "organizationalPerson", "inetOrgPerson");
    $new_user['givenName']  = "System";
    $new_user['sn']  = "Administrator";
    $new_user['cn']  = "System Administrator-" . $uid;
    $new_user['uid'] = $uid;
    $new_user['userPassword'] = $hash;

    $ldap->cd($base);
    $ldap->create_missing_trees(0, preg_replace("/^[^,]+,/", "", $dn));
    $ldap->cd($dn);
    $ldap->add($new_user);

    /* Get current base attributes */
    $ldap->cd($base);
    $ldap->cat(0, $base, array("dn", "objectClass", "gosaAclEntry"));
    $attrs = $ldap->fetch(0);

    /* Add acls for the selcted user to the base */
    $attrs_new = array();
    $attrs_new['objectClass'] = array("gosaACL");

    for ($i = 0; $i < $attrs['objectClass']['count']; $i++) {
        if (!in_array_ics($attrs['objectClass'][$i], $attrs_new['objectClass'])) {
            $attrs_new['objectClass'][] = $attrs['objectClass'][$i];
        }
    }

    $acl = "0:psub:" . base64_encode($dn) . ":all/all;cmdrw";
    $attrs_new['gosaAclEntry'][] = $acl;
    if (isset($attrs['gosaAclEntry'])) {
        for ($i = 0; $i < $attrs['gosaAclEntry']['count']; $i++) {

            $prio = preg_replace("/[:].*$/", "", $attrs['gosaAclEntry'][$i]);
            $rest = preg_replace("/^[^:]/", "", $attrs['gosaAclEntry'][$i]);

            $data = ($prio + 1) . $rest;
            $attrs_new['gosaAclEntry'][] = $data;
        }
    }

    $ldap->cd($base);
    $ldap->modify($attrs_new);        
}
