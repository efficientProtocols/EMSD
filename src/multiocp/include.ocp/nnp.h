/*
 *  This file is part of the Open C Platform (OCP) Library. OCP is a
 *  portable library for development of Data Communication Applications.
 * 
 *  Copyright (C) 1995 Neda Communications, Inc.
 * 	Prepared by Mohsen Banan (mohsen@neda.com)
 * 
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.  This library is
 *  distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 *  License for more details.  You should have received a copy of the GNU
 *  Library General Public License along with this library; if not, write
 *  to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 *  USA.
 * 
 */

/*
 * Author: Derrell Lipman
 * History:
 *
 */

#if 0

;;;
;;; Neda Message Center Subscriber Profile Definitions
;;;

;;;
;;; TODO

;;; - need a mechanism for indicating the required/optional nature of
;;; fields in various roles.  E.g., to denote that when some field is
;;; used by `ivrmsg', it is optional whereas if used by `emsd-mts' it
;;; is mandatory


;;;
;;; Syntaxes
;;;

(define-syntax BigUInt "e.g. `941213124142423566'.")
(define-syntax ColumnName "Name of a column in some table.")
(define-syntax DisplayString "Human-readable character sequence.")
(define-syntax DotDashInt "digit string with embedded `.', `-' and ` '")
(define-syntax EmailAddress "e.g., `Fletch Holmquist <fletch@emsd.neda.com>'.")
(define-syntax FaxAddress "e.g., `Fax Fletch <fletch/5629591@fax.emsd.neda.com>'.")
(define-syntax IpAddress "4 octets in dotted representation (e.g., `198.51.194.1').")
(define-syntax PagerAddress "e.g., `Pager Fletch <18007596366/1882263@pager.emsd.neda.com>'.")
(define-syntax SMSAddress "e.g., `Pager Fletch <18007596366/1882263@sms.emsd.neda.com>'.")
(define-syntax RelativeFileName "A file name without a leading `/'.")

;;;
;;; Tables
;;;

(define-table EMSD-Device
  (:columns
   (EMSD-Device-Type (:syntax DisplayString)
		    (:init :non-null)
		    (:default)
		    (:description )
		    (:used-by-programs "emsd-mts"))

   (EMSD-Protocol-Version (:syntax )
			 (:init :non-null)
			 (:default )
			 (:description )
			 (:used-by-programs "emsd-mts"))

   (EMSD-Delivery-Mode (:syntax (oneof Efficient Complete)) ; for now.. no syntax defined yet
		      (:init :non-null)
		      (:default )
		      (:description )
		      (:used-by-programs "emsd-mts"))

   (EMSD-Serialized-Messages (:syntax)
			    (:init :non-null)
			    (:default)
			    (:description)
			    (:used-by-programs "emsd-mts"))
   )
  (:primary-key (EMSD-Device-Type))
  (:foreign-keys)
  )  


(define-table Subscriber
  (:columns
   ;;
   ;; General Usage Category
   ;; 
   (Subscriber-ID (:syntax DotDashInt)
		  (:init :non-null)
		  (:default)
		  (:description "Identifies a Neda Message Center subscriber.")
		  (:used-by-programs "ivrmsg"))

   (Subscriber-Password (:syntax BigUInt)
			(:init :non-null)
			(:default 1234)
			(:description "Used for authentication.")
			(:used-by-programs))

   (Subscriber-Intl-Phone (:syntax DotDashInt)
			  (:init)
			  (:default)
			  (:description "International telephone number.")
			  (:used-by-programs "ivrmsg"))

   (Subscriber-NANP-Phone (:syntax DotDashInt)
			  (:init)
			  (:default)
			  (:description "North American Numbering Plan telephone number.")
			  (:used-by-programs "ivrmsg"))
   ;;
   ;; EMSD Usage Category
   ;; 
   (EMSD-AUA (:syntax DotDashInt)
	    (:init :non-null)
	    (:default)
	    (:description "Subscriber's (A)uthorized (U)ser (A)ddress via EMSD.")
	    (:used-by-programs "emsd-mts"))

   (EMSD-AUA-Nickname (:syntax DisplayString)
		     (:init)
		     (:cardinality n)
		     (:default)
		     (:description "First alternate name for subscriber.")
		     (:used-by-programs "emsd-mts"))

   (EMSD-Device-Type (:syntax DisplayString)
		    (:init :non-null)
		    (:default Neda-UA-v1.1)
		    (:description "Identifies the subscriber's EMSD device.  See the `EMSD-Device' table.")
		    (:used-by-programs "emsd-mts"))

   (EMSD-Reply-To (:syntax EmailAddress)
		 (:init)
		 (:default)
		 (:description "Subscriber's `Reply-to:' field when using EMSD.")
		 (:used-by-programs "emsd-mts"))

   (EMSD-UA-NSAP (:syntax IpAddress)
		(:init :non-null)
		(:default)
		(:description "IP address of the subscriber's EMSD device.")
		(:used-by-programs "emsd-mts"))

   (EMSD-UA-Password (:syntax BigUInt)
		    (:init)
		    (:default 1234)
		    (:description "Subscriber's password when using EMSD.") ; is this correct??
		    (:used-by-programs "emsd-mts"))
   ;;
   ;; IVR Usage Category
   ;; 
   (IVR-Default-AUA (:syntax ColumnName)
		    (:init :non-null)
		    (:default IVR-Email-AUA)
		    (:description "one of `IVR-Email-AUA', `IVR-Fax-AUA', `IVR-Pager-AUA'.")
		    (:used-by-programs "ivrmsg"))

   (IVR-Email-AUA (:syntax EmailAddress)
		  (:init)
		  (:default)
		  (:description "Subscribers Email address when using IVR.")
		  (:used-by-programs "ivrmsg"))

   (IVR-Fax-AUA (:syntax FaxAddress)
		(:init)
		(:default)
		(:description "Subscriber's FAX address when using IVR.")
		(:used-by-programs "ivrmsg"))

   (IVR-Pager-AUA (:syntax PagerAddress)
		  (:init :non-null)
		  (:default)
		  (:description "Subscriber's Pager address when using IVR.")
		  (:used-by-programs "ivrmsg"))

   (IVR-EMSD-AUA (:syntax)
		(:init :non-null)
		(:default)
		(:description "Subscriber's EMSD address when using IVR.")
		(:used-by-programs "ivrmsg"))

   (IVR-PocketNet-AUA (:syntax EmailAddress)
		      (:init :non-null)
		      (:default)
		      (:description "Subscriber's PocketNet address when using IVR.")
		      (:used-by-programs "ivrmsg"))

   (IVR-SMS-AUA (:syntax SMSAddress)
		(:init :non-null)
		(:default)
		(:description "Subscriber's SMS address when using IVR.")
		(:used-by-programs "ivrmsg"))

   (IVR-VOX-Name (:syntax RelativeFileName)
		 (:init)
		 (:default)
		 (:description "Name of an audio file of the subscriber's name.")
		 (:used-by-programs "ivrmsg"))
   )
  (:primary-key (Subscriber-ID))
  (:foreign-keys (EMSD-Device-Type (EMSD-Devices EMSD-Device-Type))
		 )
  )

##################################################################################
###
### Neda Message Center Subscriber Profile
###
### RCS: $Id: nnp.h,v 1.9 2002/10/25 08:37:54 mohsen Exp $
###

[EMSD-Device]
    EMSD-Device-Type = Default
    EMSD-Protocol-Version = 1.1
    EMSD-Delivery-Mode = Efficient
    EMSD-Serialized-Messages = No

[EMSD-Device]
    EMSD-Device-Type = Neda-UA-v1.1
    EMSD-Protocol-Version = 1.1
    EMSD-Delivery-Mode = Complete
    EMSD-Serialized-Messages = No

[EMSD-Device]
    EMSD-Device-Type = Neda-UA-v1.0
    EMSD-Protocol-Version = 1.0
    EMSD-Delivery-Mode = Complete
    EMSD-Serialized-Messages = Yes

[EMSD-Device]
    EMSD-Device-Type = PCSI
    EMSD-Protocol-Version = 1.1
    EMSD-Delivery-Mode = Efficient
    EMSD-Serialized-Messages = Yes

[Subscriber]
    Subscriber-ID = 201.020
    Subscriber-Password = 1234
    Subscriber-Intl-Phone = 1-206-644-8026
    Subscriber-NANP-Phone = 206-644-8026
    EMSD-AUA = 201.020
    EMSD-AUA-Nickname-1 = Fletch.Holmquist
    EMSD-AUA-Nickname-2 = fletch
    EMSD-Device-Type = Neda-UA-v1.1
    EMSD-Reply-To = Fletch Holmquist <fletch@emsd.neda.com>
    EMSD-UA-NSAP = 198.90.202.130
    EMSD-UA-Password = 1234
    IVR-Default-AUA = Fletch Holmquist <fletch@emsd.neda.com>
    IVR-Email-AUA = fletch@emsd.neda.com
    IVR-Fax-AUA = Fax Fletch <fletch/5629591@fax.emsd.neda.com>
    IVR-Pager-AUA = Pager Fletch <18007596366/1882263@pager.emsd.neda.com>
    IVR-EMSD-AUA = Fletch Holmquist <201.020@emsd.emsd.neda.com>
    IVR-VOX-Name = fletch.holmquist.vox
#   Full-Name = HRH Fletcher Q. Holmquist III
#   Postal-Address = 17005 S.E. 31st Place
#   City = Bellevue
#   State = Wa
#   Zip = 98008-5765
#   Comment-1 = first-entry
#   Comment-2 = second-entry

#endif /* 0 */


/*
 * Neda Numbering Plan (NNP) attribute names and types.
 */

#ifndef __NNP_H__
#define	__NNP_H__

#define	P_NAME_SUBSCRIBER_ID		    "Subscriber-ID"
#define	P_TYPE_SUBSCRIBER_ID		    (1)

#define	P_NAME_SUBSCRIBER_PASSWORD	    "Subscriber-Password"
#define	P_TYPE_SUBSCRIBER_PASSWORD	    (2)

#define	P_NAME_INTERNATIONAL_PHONE	    "Subscriber-Intl-Phone"
#define	P_TYPE_INTERNATIONAL_PHONE	    (3)

#define	P_NAME_NANP			    "Subscriber-NANP-Phone"
#define	P_TYPE_NANP			    (4)

#define	P_NAME_EMSD_AUA		    	    "EMSD-AUA"
#define	P_TYPE_EMSD_AUA		    	    (5)

#define	P_NAME_EMSD_AUA_NICKNAME		    "EMSD-AUA-Nickname"
#define	P_TYPE_EMSD_AUA_NICKNAME		    (P_TYPE_EMSD_AUA | PROFILE_MULTIVALUE)

#define	P_NAME_EMSD_UA_PASSWORD		    "EMSD-UA-Password"
#define	P_TYPE_EMSD_UA_PASSWORD		    (6)

#define	P_NAME_IVR_EMSD_AUA		    "IVR-EMSD-AUA"
#define	P_TYPE_IVR_EMSD_AUA		    (7)

#define	P_NAME_EMSD_REPLY_TO		    "EMSD-Reply-To"
#define	P_TYPE_EMSD_REPLY_TO		    (8)

#define	P_NAME_EMSD_UA_NSAP		    "EMSD-UA-NSAP"
#define	P_TYPE_EMSD_UA_NSAP		    (9)

#define	P_NAME_EMSD_DEVICE_TYPE		    "EMSD-Device-Type"
#define	P_TYPE_EMSD_DEVICE_TYPE		    (10)

#define	P_NAME_EMSD_PROTOCOL_VERSION	    "EMSD-Protocol-Version"
#define	P_TYPE_EMSD_PROTOCOL_VERSION	    (11)

#define	P_NAME_EMSD_DELIVERY_MODE	    "EMSD-Delivery-Mode"
#define	P_TYPE_EMSD_DELIVERY_MODE	    (12)

#define	P_NAME_EMSD_SERIAL_MODE		    "EMSD-Serialize-Messages"
#define	P_TYPE_EMSD_SERIAL_MODE		    (13)

#define	P_NAME_IVR_DEFAULT_AUA		    "IVR-Default-AUA"
#define	P_TYPE_IVR_DEFAULT_AUA		    (14)

#define	P_NAME_IVR_EMAIL_AUA		    "IVR-Email-AUA"
#define	P_TYPE_IVR_EMAIL_AUA		    (15)

#define	P_NAME_IVR_FAX_AUA		    "IVR-Fax-AUA"
#define	P_TYPE_IVR_FAX_AUA		    (16)

#define	P_NAME_IVR_PAGER_AUA		    "IVR-Pager-AUA"
#define	P_TYPE_IVR_PAGER_AUA		    (17)

#define	P_NAME_IVR_VOX			    "IVR-Vox-Name"
#define	P_TYPE_IVR_VOX			    (18)

#define	P_NAME_IVR_POCKETNET_AUA	    "IVR-PocketNet-AUA"
#define	P_TYPE_IVR_POCKETNET_AUA	    (19)

#define	P_NAME_IVR_SMS_AUA	    	    "IVR-SMS-AUA"
#define	P_TYPE_IVR_SMS_AUA	    	    (20)

#endif /* __NNP_H__ */
