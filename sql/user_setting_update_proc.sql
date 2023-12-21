
DELIMITER $$

CREATE OR REPLACE PROCEDURE user_settings_update_proc
(
 app VARCHAR(50),
 usr VARCHAR(16),
 stn VARCHAR(120),
 val VARCHAR(256)
 )
 
 BEGIN
 
 DECLARE numrows INT;

 SELECT COUNT(*) INTO numrows FROM user_settings WHERE app_name=app AND user_name=usr AND setting_name=stn;
 if numrows > 0 THEN
   UPDATE user_settings SET setting_value=val
   WHERE app_name=app AND user_name=usr AND setting_name=stn;
 ELSE
   INSERT INTO user_settings 
	  (app_name, user_name, setting_name, setting_value) 
   VALUES 
	  (app, usr, stn, val);
 END IF;

END$$

delimiter ;