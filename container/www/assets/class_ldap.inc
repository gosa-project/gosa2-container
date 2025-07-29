<?php
/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2003-2008 GONICUS GmbH
 * Copyright (C) 2003 Alejandro Escanero Blanco <aescanero@chaosdimension.org>
 * Copyright (C) 1998  Eric Kilfoil <eric@ipass.net>
 *
 * ID: $$Id$$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

define("ALREADY_EXISTING_ENTRY",-10001);
define("UNKNOWN_TOKEN_IN_LDIF_FILE",-10002);
define("NO_FILE_UPLOADED",10003);
define("INSERT_OK",10000);
define("SPECIALS_OVERRIDE", TRUE);

function in_array_ics($needle, $haystack)
{
    return in_array(strtolower($needle), array_map('strtolower', $haystack));
}

function in_array_strict($needle, $haystack, $strict = TRUE)
{
    return (in_array($needle, $haystack, $strict));
}

/*! \brief  Detect the special character handling for the currently used ldap database. 
 *          For example some convert , to \2C or " to \22.
 *         
 *  @param      Config  The GOsa configuration object.
 *  @return     Array   An array containing a character mapping the use.
 */
function detectLdapSpecialCharHandling()
{
    // The list of chars to test for
    global $config;
    if(!$config) return(NULL);

    // In the DN we've to use escaped characters, but the object name (o)
    //  has the be un-escaped.
    $name = 'GOsaLdapEncoding_,_"_(_)_+_/';
    $dnName = 'GOsaLdapEncoding_\,_\"_(_)_\+_/';
   
    // Prapare name to be useable in filters
    $fixed= LDAP::escapeValue(str_replace('\\\\', '\\\\\\', $name));
    $filterName = str_replace('\\,', '\\\\,', $fixed);
 
    // Create the target dn
    $oDN = "o={$dnName},".$config->current['BASE'];

    // Get ldap connection and check if we've already created the character 
    //  detection object. 
    $ldapCID = ldap_connect($config->current['SERVER']);
    ldap_set_option($ldapCID, LDAP_OPT_PROTOCOL_VERSION, 3);
    ldap_bind($ldapCID, $config->current['ADMINDN'],$config->current['ADMINPASSWORD']);
    $res = ldap_list($ldapCID, $config->current['BASE'], 
            "(&(o=".$filterName.")(objectClass=organization))",
            array('dn'));

    // If we haven't created the character-detection object, then create it now.
    $cnt = ldap_count_entries($ldapCID, $res);
    if(!$cnt){
        $obj = array();
        $obj['objectClass'] = array('top','organization');
        $obj['o'] = $name;
        $obj['description'] = 'GOsa character encoding test-object.';
        if(!@ldap_add($ldapCID, $oDN, $obj)){
            trigger_error("GOsa couldn't detect the special character handling used by your ldap!");
            return(NULL);
        }
    }
    
    // Read the character-handling detection entry from the ldap.
    $res = ldap_list($ldapCID, $config->current['BASE'],
            "(&(o=".$filterName.")(objectClass=organization))",
            array('dn','o'));
    $cnt = ldap_count_entries($ldapCID, $res);
    if($cnt != 1 || !$res){
        trigger_error("GOsa couldn't detect the special character handling used by your ldap!");
        return(NULL);
    }else{

        // Get the character handling entry from the ldap and check how the 
        //  values were written. Compare them with what
        //  we've initially intended to write and create a mapping out 
        //  of the results.
        $re = ldap_first_entry($ldapCID, $res);
        $attrs = ldap_get_attributes($ldapCID, $re);
   
        // Extract the interessting characters out of the dn and the 
        //  initially used $name for the entry. 
        $mapDNstr = preg_replace("/^o=GOsaLdapEncoding_(.*),".preg_quote($config->current['BASE'])."$/","\\1", trim(ldap_get_dn($ldapCID, $re)));
        $mapDN = preg_split("/_/", $mapDNstr,0, PREG_SPLIT_NO_EMPTY);

        $mapNameStr = preg_replace("/^GOsaLdapEncoding_/","",$dnName);
        $mapName = preg_split("/_/", $mapNameStr,0, PREG_SPLIT_NO_EMPTY);

        // Create a mapping out of the results.
        $map = array();
        foreach($mapName as $key => $entry){
            $map[$entry] = $mapDN[$key];
        }
        return($map);
    }
    return(NULL);
}

/*! \brief Explode a DN into its parts
 *
 * Similar to explode (http://php.net/explode), but a bit more specific
 * for the needs when splitting, exploding LDAP DNs.
 *
 * \param string 'dn' the DN to split
 * \param config-object a config object. only neeeded if DN shall be verified in the LDAP
 * \param boolean verify_in_ldap check weither DN is valid
 *
 */
function gosa_ldap_explode_dn($dn,$config = NULL,$verify_in_ldap=false)
{
  /* Initialize variables */
  $ret  = array("count" => 0);  // Set count to 0
  $next = true;                 // if false, then skip next loops and return
  $cnt  = 0;                    // Current number of loops
  $max  = 100;                  // Just for security, prevent looops
  $ldap = NULL;                 // To check if created result a valid
  $keep = "";                   // save last failed parse string

  /* Check each parsed dn in ldap ? */
  if($config!==NULL && $verify_in_ldap){
    $ldap = $config->get_ldap_link();
  }

  /* Lets start */
  $called = false;
  while(preg_match("/,/",$dn) && $next &&  $cnt < $max){

    $cnt ++;
    if(!preg_match("/,/",$dn)){
      $next = false;
    }
    $object = preg_replace("/[,].*$/","",$dn);
    $dn     = preg_replace("/^[^,]+,/","",$dn);

    $called = true;

    /* Check if current dn is valid */
    if($ldap!==NULL){
      $ldap->cd($dn);
      $ldap->cat($dn,array("dn"));
      if($ldap->count()){
        $ret[]  = $keep.$object;
        $keep   = "";
      }else{
        $keep  .= $object.",";
      }
    }else{
      $ret[]  = $keep.$object;
      $keep   = "";
    }
  }

  /* No dn was posted */
  if($cnt == 0 && !empty($dn)){
    $ret[] = $dn;
  }

  /* Append the rest */
  $test = $keep.$dn;
  if($called && !empty($test)){
    $ret[] = $keep.$dn;
  }
  $ret['count'] = count($ret) - 1;

  return($ret);
}

class LDAP
{
    public static $characterMap = NULL;
    public static $characterMapRegFrom = NULL;
    public static $characterMapRegTo = NULL;
    public static $readableMapRegFrom = NULL;
    public static $readableMapRegTo = NULL;

    var $hascon   =false;
    var $reconnect=false;
    var $tls      = false;
    static $cid;
    var $hasres   = array();
    static $sr    = array();
    var $re       = array();
    var $basedn   ="";
    var $start    = array(); // 0 if we are fetching the first entry, otherwise 1
    var $error    = ""; // Any error messages to be returned can be put here
    var $srp      = 0;
    var $objectClasses = array(); // Information read from slapd.oc.conf
    var $binddn   = "";
    var $bindpw   = "";
    var $hostname = "";
    var $follow_referral = FALSE;
    var $referrals= array();
    var $max_ldap_query_time = 0;   // 0, empty or negative values will disable this check 

    function __construct($binddn,$bindpw, $hostname, $follow_referral= FALSE, $tls= FALSE)
    {
        $this->follow_referral= $follow_referral;
        $this->tls=$tls;
        $this->binddn=self::convert($binddn);

        $this->bindpw=$bindpw;
        $this->hostname=$hostname;

        $this->connect();
    }


    function getSearchResource()
    {
        self::$sr[$this->srp]= NULL;
        $this->start[$this->srp]= 0;
        $this->hasres[$this->srp]= false;
        return $this->srp++;
    }

    /* Function to replace all problematic characters inside a DN by \001XX, where
       \001 is decoded to chr(1) [ctrl+a]. It is not impossible, but very unlikely
       that this character is inside a DN.

       Currently used codes:
       ,   => CO
       \2C => CO
       (   => OB
       )   => CB
       /   => SL                                                                  
       \22 => DQ                                                                  */
       static function convert($dn)
       {
           if (SPECIALS_OVERRIDE == TRUE){
               $tmp= preg_replace(array("/\\\\,/", "/\\\\2C/", "/\(/", "/\)/", "/\//", "/\\\\22/", '/\\\\"/'),
                       array("\001CO", "\001CO", "\001OB", "\001CB", "\001SL", "\001DQ", "\001DQ"),
                       $dn);
               return (preg_replace('/,\s+/', ',', $tmp));
           } else {
               return ($dn);
           }
       }


    /* \brief     Tests for the special-char handling of the currently used ldap database 
     *             and updates the LDAP class correspondingly.
     *            This affects the self::fix function and allows us to write 
     *             dns containing  , " ( )
     */
    static function updateSpecialCharHandling()
    {
        // Set a default character handling.
        self::$characterMapRegFrom = array("/\001CO/", "/\001OB/", "/\001CB/", "/\001SL/", "/\001DQ/");
        self::$characterMapRegTo = array("\,", "(", ")", "/", '\"');

        // Set a default mapping to make readable strings out of dns.
        self::$readableMapRegFrom = array("/\001CO/", "/\001OB/", "/\001CB/", "/\001SL/", "/\001DQ/",
                "/\\\\,/", "/\\\\2C/", "/\(/", "/\)/", "/\//", "/\\\\22/", '/\\\\"/');
        self::$readableMapRegTo = array(",", "(", ")", "/", '"',",", ",", "(", ")", "/", '"', '"');

        if(self::$characterMap === NULL){
            self::$characterMap = detectLdapSpecialCharHandling();

            // Check if character-detection was successfull, if it wasn't use a fallback.
            if(self::$characterMap !== NULL){
                self::$characterMapRegFrom = array();
                self::$characterMapRegTo = array();
                self::$readableMapRegFrom = array();
                self::$readableMapRegTo = array();
                foreach(self::$characterMap as $from => $to){
                   
                    // Append entry character conversion  
                    self::$characterMapRegFrom[] = "/".preg_quote(self::convert($from),'/')."/";
                    self::$characterMapRegTo[] = addslashes($to);

                    // Append entry make readable array
                    self::$readableMapRegFrom[] = "/".preg_quote($to,'/')."/";
                    self::$readableMapRegFrom[] = "/".preg_quote(self::convert($to),'/')."/";
                    self::$readableMapRegTo[] = stripslashes($from);
                    self::$readableMapRegTo[] = stripslashes($from);
                }
            }else{
                // To avoid querying a hundred times without any success, stop here.
                self::$characterMap = array();
            }
        }
        
        // Store results in the session.
        $attrs = array("characterMap", "characterMapRegFrom", "characterMapRegTo", "readableMapRegFrom", "readableMapRegTo");
        $data = array();
        foreach($attrs as $attr){
            $data[$attr] = self::$$attr;
        }

        $attrs = array(implode(', ', self::$characterMapRegFrom),implode(', ', self::$characterMapRegTo));
    }


    /*!  \brief     This methods replaces non-readable characters be readable ones, 
     *               depends on what were detect in 'updateSpecialCharHandling'.
     * @param       String  The DN/string to cleanup 
     * @return      String  The readable string.
     */
    static function makeReadable($dn)
    {
        if(self::$characterMap === NULL) self::updateSpecialCharHandling();
        return (preg_replace(self::$readableMapRegFrom,self::$readableMapRegTo,$dn));
    }


    /* \brief   Function to fix all problematic characters inside a DN by replacing \001XX
     *           codes to their original values. See "convert" for more information. 
     *          The ',' characters are always expanded to \, (not \2C), since all tested LDAP
     *           servers seem to take it the correct way.                                 
     * @param String  The DN to convert characters in. 
     * @param String  The converted dn.
     */
    static function fix($dn)
    {
        if (SPECIALS_OVERRIDE == TRUE && $dn != ""){

            // Update the conversion instruction set.
            if(self::$characterMap === NULL) self::updateSpecialCharHandling();

            return (preg_replace(self::$characterMapRegFrom,self::$characterMapRegTo,$dn));
        } else {
            return ($dn);
        }
    }


    /* Function to fix problematic characters in DN's that are used for search
       requests. I.e. member=....                                               */
    static function prepare4filter($dn)
    {
        $fixed= LDAP::escapeValue(str_replace('\\\\', '\\\\\\', self::fix($dn)));
        return str_replace('\\,', '\\\\,', $fixed);
    }


    function connect()
    {
        $this->hascon=false;
        $this->reconnect=false;
        if (self::$cid= @ldap_connect($this->hostname)) {
            @ldap_set_option(self::$cid, LDAP_OPT_PROTOCOL_VERSION, 3);
            if (function_exists("ldap_set_rebind_proc") && $this->follow_referral) {
                @ldap_set_option(self::$cid, LDAP_OPT_REFERRALS, 1);
                @ldap_set_rebind_proc(self::$cid, array(&$this, "rebind"));
            }
            if (function_exists("ldap_start_tls") && $this->tls){
                @ldap_start_tls(self::$cid);
            }

            $this->error = "No Error";
            if ($bid = @ldap_bind(self::$cid, self::fix($this->binddn), $this->bindpw)) {
                $this->error = "Success";
                $this->hascon=true;
            } else {
                if ($this->reconnect){
                    if ($this->error != "Success"){
                        $this->error = "Could not rebind to " . $this->binddn;
                    }
                } else {
                    $this->error = "Could not bind to " . $this->binddn;
                }
            }
        } else {
            $this->error = "Could not connect to LDAP server";
        }
    }

    function rebind($ldap, $referral)
    {
        $credentials= $this->get_credentials($referral);
        if (@ldap_bind($ldap, self::fix($credentials['ADMINDN']), $credentials['ADMINPASSWORD'])) {
            $this->error = "Success";
            $this->hascon=true;
            $this->reconnect= true;
            return (0);
        } else {
            $this->error = "Could not bind to " . $credentials['ADMINDN'];
            return NULL;
        }
    }

    function reconnect()
    {
        if ($this->reconnect){
            @ldap_unbind(self::$cid);
            self::$cid = NULL;
        }
    }

    function unbind()
    {
        @ldap_unbind(self::$cid);
        self::$cid = NULL;
    }

    function disconnect()
    {
        if($this->hascon){
            @ldap_close(self::$cid);
            $this->hascon=false;
        }
    }

    function cd($dir)
    {
        if ($dir == ".."){
            $this->basedn = $this->getParentDir();
        } else {
            $this->basedn = self::convert($dir);
        }
    }

    function getParentDir($basedn = "")
    {
        if ($basedn==""){
            $basedn = $this->basedn;
        } else {
            $basedn = self::convert($basedn);
        }
        return(preg_replace("/[^,]*[,]*[ ]*(.*)/", "$1", $basedn));
    }


    function search($srp, $filter, $attrs= array())
    {
        if($this->hascon){
            if ($this->reconnect) $this->connect();

            $start = microtime(true);
            $this->clearResult($srp);
            self::$sr[$srp] = @ldap_search(self::$cid, self::fix($this->basedn), $filter, $attrs);
            $this->error = @ldap_error(self::$cid);
            $this->resetResult($srp);
            $this->hasres[$srp]=true;

            /* Check if query took longer as specified in max_ldap_query_time */
            if($this->max_ldap_query_time){
                $diff = microtime(true) - $start;
                if($diff > $this->max_ldap_query_time){
                    msg_dialog::display(_("Performance warning"), sprintf(_("LDAP performance is poor: last query took %.2fs!"), $diff), WARNING_DIALOG);
                }
            }

            $this->log("LDAP operation: time=".(microtime(true)-$start)." operation=search('".self::fix($this->basedn)."', '$filter')");

            // Create statistic table entry 
            return(self::$sr[$srp]);
        }else{
            $this->error = "Could not connect to LDAP server";
            return("");
        }
    }

    function ls($srp, $filter = "(objectclass=*)", $basedn = "",$attrs = array("*"))
    {
        if($this->hascon){
            if ($this->reconnect) $this->connect();

            $this->clearResult($srp);
            if ($basedn == "")
                $basedn = $this->basedn;
            else
                $basedn= self::convert($basedn);

            $start = microtime(true);
            self::$sr[$srp] = @ldap_list(self::$cid, self::fix($basedn), $filter,$attrs);
            $this->error = @ldap_error(self::$cid);
            $this->resetResult($srp);
            $this->hasres[$srp]=true;

            /* Check if query took longer as specified in max_ldap_query_time */
            if($this->max_ldap_query_time){
                $diff = microtime(true) - $start;
                if($diff > $this->max_ldap_query_time){
                    msg_dialog::display(_("Performance warning"), sprintf(_("LDAP performance is poor: last query took %.2fs!"), $diff), WARNING_DIALOG);
                }
            }

            $this->log("LDAP operation: time=".(microtime(true) - $start)." operation=ls('".self::fix($basedn)."', '$filter')");

            return(self::$sr[$srp]);
        }else{
            $this->error = "Could not connect to LDAP server";
            return("");
        }
    }

    function cat($srp, $dn,$attrs= array("*"), $filter = "(objectclass=*)")
    {
        if($this->hascon){
            if ($this->reconnect) $this->connect();
            
            if (self::fix($dn) == "new") $dn = $this->basedn;


            $this->clearResult($srp);
            self::$sr[$srp] = @ldap_read(self::$cid, self::fix($dn), $filter,$attrs);
            
            $this->error = @ldap_error(self::$cid);
            $this->resetResult($srp);
            $this->hasres[$srp]=true;
            return(self::$sr[$srp]);
        }else{
            $this->error = "Could not connect to LDAP server";
            return("");
        }
    }

    function object_match_filter($dn,$filter)
    {
        if($this->hascon){
            if ($this->reconnect) $this->connect();
            $res =  @ldap_read(self::$cid, self::fix($dn), $filter, array("objectClass"));
            $rv =   @ldap_count_entries(self::$cid, $res);
            return($rv);
        }else{
            $this->error = "Could not connect to LDAP server";
            return(FALSE);
        }
    }

    function set_size_limit($size)
    {
        /* Ignore zero settings */
        if ($size == 0){
            @ldap_set_option(self::$cid, LDAP_OPT_SIZELIMIT, 10000000);
        }
        if($this->hascon){
            @ldap_set_option(self::$cid, LDAP_OPT_SIZELIMIT, $size);
        } else {
            $this->error = "Could not connect to LDAP server";
        }
    }

    function fetch($srp)
    {
        $att= array();
        if($this->hascon){
            if($this->hasres[$srp]){
                if ($this->start[$srp] == 0)
                {
                    if (self::$sr[$srp]){
                        $this->start[$srp] = 1;
                        $this->re[$srp]= @ldap_first_entry(self::$cid, self::$sr[$srp]);
                    } else {
                        return array();
                    }
                } else {
                    $this->re[$srp]= @ldap_next_entry(self::$cid, $this->re[$srp]);
                }
                if ($this->re[$srp])
                {
                    $att= @ldap_get_attributes(self::$cid, $this->re[$srp]);
                    $att['dn']= trim(self::convert(@ldap_get_dn(self::$cid, $this->re[$srp])));
                }
                $this->error = @ldap_error(self::$cid);
                if (!isset($att)){
                    $att= array();
                }
                return($att);
            }else{
                $this->error = "Perform a fetch with no search";
                return("");
            }
        }else{
            $this->error = "Could not connect to LDAP server";
            return("");
        }
    }

    function resetResult($srp)
    {
        $this->start[$srp] = 0;
    }

    function clearResult($srp)
    {
        if($this->hasres[$srp]){
            $this->hasres[$srp] = false;
            if (!is_bool(self::$sr[$srp])) {
                @ldap_free_result(self::$sr[$srp]);
            }
        }
    }

    function getDN($srp)
    {
        if($this->hascon){
            if($this->hasres[$srp]){

                if(!$this->re[$srp])
                {
                    $this->error = "Perform a Fetch with no valid Result";
                }
                else
                {
                    $rv = @ldap_get_dn(self::$cid, $this->re[$srp]);

                    $this->error = @ldap_error(self::$cid);
                    return(trim(self::convert($rv)));
                }
            }else{
                $this->error = "Perform a Fetch with no Search";
                return("");
            }
        }else{
            $this->error = "Could not connect to LDAP server";
            return("");
        }
    }

    function count($srp)
    {
        if($this->hascon){
            if($this->hasres[$srp]){
                $rv = @ldap_count_entries(self::$cid, self::$sr[$srp]);
                $this->error = @ldap_error(self::$cid);
                return($rv);
            }else{
                $this->error = "Perform a Fetch with no Search";
                return("");
            }
        }else{
            $this->error = "Could not connect to LDAP server";
            return("");
        }
    }

    function rm($attrs = array(), $dn = "")
    {
        if($this->hascon){
            if ($this->reconnect) $this->connect();
            if ($dn == "")
                $dn = $this->basedn;

            $r = ldap_mod_del(self::$cid, self::fix($dn), $attrs);
            $this->error = @ldap_error(self::$cid);
            return($r);
        }else{
            $this->error = "Could not connect to LDAP server";
            return("");
        }
    }

    function mod_add($attrs = array(), $dn = "")
    {
        print($attrs);

        if($this->hascon){
            if ($this->reconnect) $this->connect();
            if ($dn == "")
                $dn = $this->basedn;

            $r = @ldap_mod_add(self::$cid, self::fix($dn), $attrs);
            $this->error = @ldap_error(self::$cid);
            return($r);
        }else{
            $this->error = "Could not connect to LDAP server";
            return("");
        }
    }

    function rename($attrs, $dn = "")
    {
        if($this->hascon){
            if ($this->reconnect) $this->connect();
            if ($dn == "")
                $dn = $this->basedn;

            $r = @ldap_mod_replace(self::$cid, self::fix($dn), $attrs);
            $this->error = @ldap_error(self::$cid);
            return($r);
        }else{
            $this->error = "Could not connect to LDAP server";
            return("");
        }
    }

    function rmdir($deletedn)
    {
        if($this->hascon){
            if ($this->reconnect) $this->connect();
            $r = @ldap_delete(self::$cid, self::fix($deletedn));
            $this->error = @ldap_error(self::$cid);
            return($r ? $r : 0);
        }else{
            $this->error = "Could not connect to LDAP server";
            return("");
        }
    }


    /*! \brief Move the given Ldap entry from $source to $dest
      @param  String  $source The source dn.
      @param  String  $dest   The destination dn.
      @return Boolean TRUE on success else FALSE.
     */
    function rename_dn($source,$dest)
    {
        /* Check if source and destination are the same entry */
        if(strtolower($source) == strtolower($dest)){
            trigger_error("Source and destination can't be the same entry.");
            $this->error = "Source and destination can't be the same entry.";
            return(FALSE);
        }

        /* Check if destination entry exists */    
        if($this->dn_exists($dest)){
            trigger_error("Destination '$dest' already exists.");
            $this->error = "Destination '$dest' already exists.";
            return(FALSE);
        }

        /* Extract the name and the parent part out ouf source dn.
           e.g.  cn=herbert,ou=department,dc=... 
           parent   =>  ou=department,dc=...
           dest_rdn =>  cn=herbert
         */
        $parent   = preg_replace("/^[^,]+,/","", $dest);
        $dest_rdn = preg_replace("/,.*$/","",$dest);

        if($this->hascon){
            if ($this->reconnect) $this->connect();
            $r= ldap_rename(self::$cid,@self::fix($source), @self::fix($dest_rdn),@self::fix($parent),TRUE); 
            $this->error = ldap_error(self::$cid);

            /* Check if destination dn exists, if not the 
               server may not support this operation */
            $r &= is_resource($this->dn_exists($dest));
            return($r);
        }else{
            $this->error = "Could not connect to LDAP server";
            return(FALSE);
        }
    }


    /**
     *  Function rmdir_recursive
     *
     *  Description: Based in recursive_remove, adding two thing: full subtree remove, and delete own node.
     *  Parameters:  The dn to delete
     *  GiveBack:    True on sucessfull , 0 in error, and "" when we don't get a ldap conection
     *
     */
    function rmdir_recursive($srp, $deletedn)
    {
        if($this->hascon){
            if ($this->reconnect) $this->connect();
            $delarray= array();

            /* Get sorted list of dn's to delete */
            $this->ls ($srp, "(objectClass=*)",$deletedn);
            while ($this->fetch($srp)){
                $deldn= $this->getDN($srp);
                $delarray[$deldn]= strlen($deldn);
            }
            arsort ($delarray);
            reset ($delarray);

            /* Really Delete ALL dn's in subtree */
            foreach ($delarray as $key => $value){
                $this->rmdir_recursive($srp, $key);
            }

            /* Finally Delete own Node */
            $r = @ldap_delete(self::$cid, self::fix($deletedn));
            $this->error = @ldap_error(self::$cid);
            return($r ? $r : 0);
        }else{
            $this->error = "Could not connect to LDAP server";
            return("");
        }
    }

    function makeReadableErrors($error,$attrs)
    { 
        if($this->success()) return("");

        $str = "";
        if(preg_match("/^objectClass: value #([0-9]*) invalid per syntax$/", $this->get_additional_error())){
            $oc = preg_replace("/^objectClass: value #([0-9]*) invalid per syntax$/","\\1", $this->get_additional_error());
            if(isset($attrs['objectClass'][$oc])){
                $str.= " - <b>objectClass: ".$attrs['objectClass'][$oc]."</b>";
            }
        }
        if($error == "Undefined attribute type"){
            $str = " - <b>attribute: ".preg_replace("/:.*$/","",$this->get_additional_error())."</b>";
        } 


        return($str);
    }

    function modify($attrs)
    {
        if(count($attrs) == 0){
            return (0);
        }
        if($this->hascon){
            $start = microtime(TRUE);
            if ($this->reconnect) $this->connect();
            $r = @ldap_modify(self::$cid, self::fix($this->basedn), $attrs);
            $this->error = @ldap_error(self::$cid);
            if(!$this->success()){
                $this->error.= $this->makeReadableErrors($this->error,$attrs);
            }

            return($r ? $r : 0);
        }else{
            $this->error = "Could not connect to LDAP server";
            return("");
        }
    }

    function add($attrs)
    {
        if($this->hascon){
            $start = microtime(TRUE);
            if ($this->reconnect) $this->connect();
            $r = @ldap_add(self::$cid, self::fix($this->basedn), $attrs);
            $this->error = @ldap_error(self::$cid);
            if(!$this->success()){
                $this->error.= $this->makeReadableErrors($this->error,$attrs);
            }

            return($r ? $r : 0);
        }else{
            $this->error = "Could not connect to LDAP server";
            return("");
        }
    }

    function create_missing_trees($srp, $target)
    {
        $real_path= substr($target, 0, strlen($target) - strlen($this->basedn) -1 );

        if ($target == $this->basedn){
            $l= array("dummy");
        } else {
            $l= array_reverse(gosa_ldap_explode_dn($real_path));
        }
        unset($l['count']);
        $cdn= $this->basedn;
        $tag= "";

        /* Load schema if available... */
        $classes= $this->get_objectclasses();

        foreach ($l as $part){
            if ($part != "dummy"){
                $cdn= "$part,$cdn";
            }

            /* Ignore referrals */
            $found= false;
            foreach($this->referrals as $ref){
                $base= preg_replace('!^[^:]+://[^/]+/([^?]+).*$!', '\\1', $ref['URI']);
                if ($base == $cdn){
                    $found= true;
                    break;
                }
            }
            if ($found){
                continue;
            }

            $this->cat ($srp, $cdn);
            $attrs= $this->fetch($srp);

            /* Create missing entry? */
            if (count ($attrs)){

                /* Catch the tag - if present */
                if (isset($attrs['gosaUnitTag'][0])){
                    $tag= $attrs['gosaUnitTag'][0];
                }

            } else {
                $type= preg_replace('/^([^=]+)=.*$/', '\\1', $cdn);
                $param= self::fix(preg_replace('/^[^=]+=([^,]+).*$/', '\\1', $cdn));
                $param=preg_replace(array('/\\\\,/','/\\\\"/'),array(',','"'),$param);

                $na= array();

                /* Automatic or traditional? */
                if(count($classes)){

                    /* Get name of first matching objectClass */
                    $ocname= "";
                    foreach($classes as $class){
                        if (isset($class['MUST']) && in_array_strict($type, $class['MUST'])){

                            /* Look for first classes that is structural... */
                            if (isset($class['STRUCTURAL'])){
                                $ocname= $class['NAME'];
                                break;
                            }

                            /* Look for classes that are auxiliary... */
                            if (isset($class['AUXILIARY'])){
                                $ocname= $class['NAME'];
                            }
                        }
                    }

                    /* Bail out, if we've nothing to do... */
                    if ($ocname == ""){
                        msg_dialog::display(_("Internal error"), sprintf(_("Cannot automatically create subtrees with RDN %s: no object class found"), bold($type)), FATAL_ERROR_DIALOG);
                        exit();
                    }

                    /* Assemble_entry */
                    if ($tag != ""){
                        $na['objectClass']= array($ocname, "gosaAdministrativeUnitTag");
                        $na["gosaUnitTag"]= $tag;
                    } else {
                        $na['objectClass']= array($ocname);
                    }
                    if (isset($classes[$ocname]['AUXILIARY'])){
                        $na['objectClass'][]= $classes[$ocname]['SUP'];
                    }
                    if ($type == "dc"){
                        /* This is bad actually, but - tell me a better way? */
                        $na['objectClass'][]= 'locality';
                    }
                    $na[$type]= $param;

                    // Fill in MUST values - but do not overwrite existing ones.
                    if (is_array($classes[$ocname]['MUST'])){
                        foreach($classes[$ocname]['MUST'] as $attr){
                            if(isset($na[$attr]) && !empty($na[$attr])) continue;
                            $na[$attr]= "filled";
                        }
                    }

                } else {

                    /* Use alternative add... */
                    switch ($type){
                        case 'ou':
                            if ($tag != ""){
                                $na["objectClass"]= array("organizationalUnit", "gosaAdministrativeUnitTag");
                                $na["gosaUnitTag"]= $tag;
                            } else {
                                $na["objectClass"]= "organizationalUnit";
                            }
                            $na["ou"]= $param;
                            break;
                        case 'dc':
                            if ($tag != ""){
                                $na["objectClass"]= array("dcObject", "top", "locality", "gosaAdministrativeUnitTag");
                                $na["gosaUnitTag"]= $tag;
                            } else {
                                $na["objectClass"]= array("dcObject", "top", "locality");
                            }
                            $na["dc"]= $param;
                            break;
                        default:
                            msg_dialog::display(_("Internal error"), sprintf(_("Cannot automatically create subtrees with RDN %s: not supported"), bold($type)), FATAL_ERROR_DIALOG);
                            exit();
                    }

                }
                $this->cd($cdn);
                $this->add($na);

                if (!$this->success()){

                    print_a(array($cdn,$na));

                    msg_dialog::display(_("LDAP error"), msgPool::ldaperror($this->get_error(), $cdn, LDAP_ADD, __CLASS__));
                    return FALSE;
                }
            }
        }

        return TRUE;
    }


    function recursive_remove($srp)
    {
        $delarray= array();

        /* Get sorted list of dn's to delete */
        $this->search ($srp, "(objectClass=*)");
        while ($this->fetch($srp)){
            $deldn= $this->getDN($srp);
            $delarray[$deldn]= strlen($deldn);
        }
        arsort ($delarray);
        reset ($delarray);

        /* Delete all dn's in subtree */
        foreach ($delarray as $key => $value){
            $this->rmdir($key);
        }
    }


    function get_attribute($dn, $name,$r_array=0)
    {
        $data= "";
        if ($this->reconnect) $this->connect();
        $sr= @ldap_read(self::$cid, self::fix($dn), "objectClass=*", array("$name"));

        /* fill data from LDAP */
        if ($sr) {
            $ei= @ldap_first_entry(self::$cid, $sr);
            if ($ei) {
                if ($info= @ldap_get_values_len(self::$cid, $ei, "$name")){
                    $data= $info[0];
                }
            }
        }
        if($r_array==0) {
            return ($data);
        } else {
            return ($info);
        }
    }



    function get_additional_error()
    {
        $error= "";
        @ldap_get_option (self::$cid, LDAP_OPT_ERROR_STRING, $error);
        return ($error);
    }


    function success()
    {
        return (trim($this->error) === 'Success');
    }


    function get_error()
    {
        if ($this->error == 'Success'){
            return $this->error;
        } else {
            $adderror= $this->get_additional_error();
            if ($adderror != ""){
                $error= $this->error." (".$this->get_additional_error().", ".sprintf(_("while operating on %s using LDAP server %s"), bold($this->basedn), bold($this->hostname)).")";
            } else {
                $error= $this->error." (".sprintf(_("while operating on LDAP server %s"), bold($this->hostname)).")";
            }
            return $error;
        }
    }

    static function get_credentials($url, $referrals= NULL)
    {
        $ret= array();
        $url= preg_replace('!\?\?.*$!', '', $url);
        $server= preg_replace('!^([^:]+://[^/]+)/.*$!', '\\1', $url);

        if ($referrals === NULL){
            $referrals= self::$referrals;
        }

        if (isset($referrals[$server])){
            return ($referrals[$server]);
        } else {
            $ret['ADMINDN']= self::fix(self::$binddn);
            $ret['ADMINPASSWORD']= self::$bindpw;
        }

        return ($ret);
    }


    /*! \brief  Generates an ldif for all entries matching the filter settings, scope and limit.
     *  @param  $dn           The entry to export.
     *  @param  $filter       Limit the exported object to those maching this filter.
     *  @param  $scope        'base', 'sub' .. see manpage for 'ldapmodify' for details.
     *  @param  $limit        Limits the result.
     */
    function generateLdif ($dn, $filter= "(objectClass=*)", $scope = 'sub', $limit=0)
    {
        // Ensure that limit is numeric if not skip here.
        if(!empty($limit) && !is_numeric($limit)){
            trigger_error(sprintf("Invalid parameter for limit '%s', a numeric value is required."), $limit);
            return(NULL);
        }
        $limit = (!$limit)?'':' -z '.$limit;

        // Check scope values
        $scope = trim($scope);
        if(!empty($scope) && !in_array_strict($scope, array('base', 'one', 'sub', 'children'))){
            trigger_error(sprintf("Invalid parameter for scope '%s', please use 'base', 'one', 'sub' or 'children'."), $scope);
            return(NULL);
        }
        $scope = (!empty($scope))?' -s '.$scope: '';

        // First check if we are able to call 'ldapsearch' on the command line.
        $check = shell_exec('which ldapsearch');
        if(empty($check)){
            $this->error = sprintf(_("Command line programm %s is missing!"), bold('ldapsearch'));
            return(NULL);
        }

        // Prepare parameters to be valid for shell execution
        $dn = escapeshellarg($dn);
        $pwd = $this->bindpw;
        $host = escapeshellarg($this->hostname);
        $admin = escapeshellarg($this->binddn);
        $filter = escapeshellarg($filter);
        $cmd = "ldapsearch -x -LLLL -D {$admin} {$filter} {$limit} {$scope} -H {$host} -b {$dn} -W ";

        // Create list of process pipes  
        $descriptorspec = array(
                0 => array("pipe", "r"),  // stdin
                1 => array("pipe", "w"),  // stdout
                2 => array("pipe", "w")); // stderr

        // Try to open the process 
        $process = proc_open($cmd, $descriptorspec, $pipes);
        if (is_resource($process)) {

            // Write the password to stdin
            fwrite($pipes[0], $pwd."\n");
            fclose($pipes[0]);

            // Get results from stdout and stderr
            $res = stream_get_contents($pipes[1]);
            $err = stream_get_contents($pipes[2]);
            fclose($pipes[1]);

            // Close the process and check its return value
            if(proc_close($process) != 0){
                $this->error = $err;
                return(NULL);
            }
        }
        return($res);
    }

    function dn_exists($dn)
    {
        return @ldap_list(self::$cid, self::fix($dn), "(objectClass=*)", array("objectClass"));
    }

    function get_objectclasses( $force_reload = FALSE)
    {
        $objectclasses = array();

        # Get base to look for schema 
        $sr = @ldap_read (self::$cid, "", "objectClass=*", array("subschemaSubentry"));
        $attr = @ldap_get_entries(self::$cid,$sr);
        if (!isset($attr[0]['subschemasubentry'][0])){
            return array();
        }

        /* Get list of objectclasses and fill array */
        $nb= $attr[0]['subschemasubentry'][0];
        $objectclasses= array();
        $sr= ldap_read (self::$cid, $nb, "objectClass=*", array("objectclasses"));
        $attrs= ldap_get_entries(self::$cid,$sr);
        if (!isset($attrs[0])){
            return array();
        }
        foreach ($attrs[0]['objectclasses'] as $val){
            if (preg_match('/^[0-9]+$/', $val)){
                continue;
            }
            $name= "OID";
            $pattern= explode(' ', $val);
            $ocname= preg_replace("/^.* NAME\s+\(*\s*'([^']+)'\s*\)*.*$/", '\\1', $val);
            $objectclasses[$ocname]= array();

            foreach($pattern as $chunk){
                switch($chunk){

                    case '(':
                        $value= "";
                        break;

                    case ')': if ($name != ""){
                                  $v = $this->value2container($value);
                                  if(in_array_strict($name, array('MUST', 'MAY')) && !is_array($v)){
                                      $v = array($v);
                                  }
                                  $objectclasses[$ocname][$name]= $v;
                              }
                              $name= "";
                              $value= "";
                              break;

                    case 'NAME':
                    case 'DESC':
                    case 'SUP':
                    case 'STRUCTURAL':
                    case 'ABSTRACT':
                    case 'AUXILIARY':
                    case 'MUST':
                    case 'MAY':
                              if ($name != ""){
                                  $v = $this->value2container($value);
                                  if(in_array_strict($name, array('MUST', 'MAY')) && !is_array($v)){
                                      $v = array($v);
                                  }
                                  $objectclasses[$ocname][$name]= $v;
                              }
                              $name= $chunk;
                              $value= "";
                              break;

                    default:  $value.= $chunk." ";
                }
            }

        }

        return $objectclasses;
    }


    function value2container($value)
    {
        /* Set emtpy values to "true" only */
        if (preg_match('/^\s*$/', $value)){
            return true;
        }

        /* Remove ' and " if needed */
        $value= preg_replace('/^[\'"]/', '', $value);
        $value= preg_replace('/[\'"] *$/', '', $value);

        /* Convert to array if $ is inside... */
        if (preg_match('/\$/', $value)){
            $container= preg_split('/\s*\$\s*/', $value);
        } else {
            $container= chop($value);
        }

        return ($container);
    }


    function log($string)
    {
        print($string);
    }

    /* added by Guido Serra aka Zeph <zeph@purotesto.it> */
    function getCn($dn){
        $simple= explode(",", $dn);

        foreach($simple as $piece) {
            $partial= explode("=", $piece);

            if($partial[0] == "cn"){
                return $partial[1];
            }
        }
    }


    static function get_naming_contexts($server, $admin= "", $password= "")
    {
        /* Build LDAP connection */
        $ds= ldap_connect ($server);
        if (!$ds) {
            die ("Can't bind to LDAP. No check possible!");
        }
        ldap_set_option($ds, LDAP_OPT_PROTOCOL_VERSION, 3);
        $r= ldap_bind ($ds, $admin, $password);

        /* Get base to look for naming contexts */
        $sr  = @ldap_read ($ds, "", "objectClass=*", array("+"));
        $attr= @ldap_get_entries($ds,$sr);

        return ($attr[0]['namingcontexts']);
    }


    function get_root_dse($server, $admin= "", $password= "")
    {
        /* Build LDAP connection */
        $ds= ldap_connect ($server);
        if (!$ds) {
            die ("Can't bind to LDAP. No check possible!");
        }
        ldap_set_option($ds, LDAP_OPT_PROTOCOL_VERSION, 3);
        $r= ldap_bind ($ds, $admin, $password);

        /* Get base to look for naming contexts */
        $sr  = @ldap_read ($ds, "", "objectClass=*", array("+"));
        $attr= @ldap_get_entries($ds,$sr);

        /* Return empty array, if nothing was set */
        if (!isset($attr[0])){
            return array();
        }

        /* Rework array... */
        $result= array();
        for ($i= 0; $i<$attr[0]['count']; $i++){
            $result[$attr[0][$i]]= $attr[0][$attr[0][$i]];
            unset($result[$attr[0][$i]]['count']);
        }

        return ($result);
    }

}
