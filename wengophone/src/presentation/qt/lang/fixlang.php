<?php


for($i = 1; $i < $argc; $i++)
{
  $fn = $argv[$i];
  $obj = simplexml_load_file($fn);
  print "Fixing {$fn}\n";
  foreach($obj->context as $ctx)
  {
   foreach($ctx->message as $msg)
   {
      $text = (string) $msg->translation;
      $text = trim($text);
      if (strstr($msg->source, "GNU"))
	$att="ATTENTION!!!!!*****";
      else
        $att="";

      print  $att."translation = ".$text." len=".strlen($text)."\r\n";
      if (!strlen($text))
	{
	  print "Fixing...\r\n";
	  $msg->translation->addAttribute("type", "unfinished");
	}
    };
  };

  $str = $obj->asXML();

  $out = fopen($fn, "w");
  fputs($out, $str);
  fclose($out);
}
?>
