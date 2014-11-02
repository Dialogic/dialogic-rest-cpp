/*
 * Copyright (C) 2014 Dialogic Corp.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 * Alternatively see <http://www.gnu.org/licenses/>.
 * Or see the LICENSE file included within the source tree.
 *
 */

#ifndef _CONFERENCE720P_H
#define _CONFERENCE720P_H

/*------------------------------ Dependencies --------------------------------*/

#include <sys/stat.h>
#include <string.h>
#include "dispatchxmscmd.h"
#include "xmseventparser.h"

/*----------------------------------------------------------------------------*/

 // class Conference720p
 // Conference at 720p resolution
class Conference720p
{
  public:
    Conference720p (std::string dtmf_mode);
    virtual ~ Conference720p ();
    //virtual void onEvent (xmsEvent *event);
    void onEvent (xmsEventParser *event);

    char *getActiveMediaOp ()
    {
        return active_media_op_;
    }

    void setActiveMediaOp (char *media_id)
    {
        strcpy (active_media_op_, media_id);
    }

    void nullActiveMediaOp ()
    {
        memset (active_media_op_, 0, sizeof (active_media_op_));
    }

    char *getExclusiveMediaOp ()
    {
        return exclusive_media_op_;
    }

    void setExclusiveMediaOp (const char *media_id)
    {
        strcpy (exclusive_media_op_, media_id);
    }

    void nullExclusiveMediaOp ()
    {
        memset (exclusive_media_op_, 0, sizeof (exclusive_media_op_));
    }

    void incNumCallers ()
    {
        num_callers_++;
    }

    void decNumCallers ()
    {
        num_callers_--;
    }

    int getNumCallers ()
    {
        return num_callers_;
    }

    char *getDtmfMode ()
    {
        return dtmf_mode_;
    }

    void setDtmfMode (char *dtmf_mode)
    {
        strcpy (dtmf_mode_, dtmf_mode);
    }

    bool isVeryFirstCall ()
    {
        return is_very_first_call_;
    }

    void didVeryFirstCall ()
    {
        is_very_first_call_ = false;
    }

    void resetConference ();

    void init_region_use ()
    {
        for (int cnt = 0; cnt < 9; cnt++)
        {
            region_use_[cnt] = false;
            region_processed_[cnt] = false;
        }
    }

    bool is_region_used (int region)
    {
        return region_use_[region - 1];
    }

    bool is_region_not_processed (int region)
    {
        return !region_processed_[region - 1];
    }

    void set_all_regions_processed ()
    {
        for (int cnt = 0; cnt < 9; cnt++)
            region_processed_[cnt] = true;
    }

    void set_all_regions_not_processed ()
    {
        for (int cnt = 0; cnt < 9; cnt++)
            region_processed_[cnt] = false;
    }

    void set_region_processed (int region)
    {
        region_processed_[region - 1] = true;
    }

    void clear_region (int region)
    {
        region_use_[region - 1] = false;
    }

    void set_region (int region)
    {
        region_use_[region - 1] = true;
    }

    int getNumActiveRegions ()
    {
        return layout_ - '0';
    }

    int get_next_open_region ()
    {
        for (int cnt = 0; cnt < 9; cnt++)
        {
            if (region_use_[cnt] == false)
            {
                region_use_[cnt] = true;
                return cnt + 1;
            }
        }
        return 0;
    }

    void setRecordInProgress ()
    {
        recordInProgress_ = true;
    }

    void setRecordNotInProgress ()
    {
        recordInProgress_ = false;
    }

    bool isRecordInProgress ()
    {
        return recordInProgress_;
    }

    void setScrollingOverlayOn ()
    {
        scrolling_overlay_ = true;
    }

    void setScrollingOverlayOff ()
    {
        scrolling_overlay_ = false;
    }

    bool scrollingOverlayOn ()
    {
        return scrolling_overlay_;
    }

    void setSlideShowOn ()
    {
        slide_show_ = true;
    }

    void setSlideShowOff ()
    {
        slide_show_ = false;
    }

    bool slideShowOn ()
    {
        return slide_show_;
    }

    bool areCaptionsOn ()
    {
        return captionsOn_;
    }

    void setCaptionsOn ()
    {
        captionsOn_ = true;
    }

    void setCaptionsOff ()
    {
        captionsOn_ = false;
    }

    int get_cur_layout ()
    {
        return layout_ - '0';
    }


    // More complicated functions in test.cpp
    void rotate_to_next_region ();
    int get_next_region (const int region);
    void move_caption (const int oldRegion, const int newRegion);
    void notify_all_callers (const char *message);
    bool is_region_max_for_layout (const int region);
    void turnOnAllCaptions ();
    void turnOffAllCaptions ();
    void turnOnCaption (int region);
    void turnOffCaption (int region);
    void turnOffVideoLabels ();
    int find_clicked_region (const char *resolution, int layout, int posX, int posY);
	void resetDemo();
    void moveRegionOverlays (const int fromRegion, const int toRegion);
    void copyRegionOverlays (const int fromRegion, const int toRegion);


  private:

    char active_media_op_[50];
    char exclusive_media_op_[50];

    // Counter to limit callers in any given app
    int num_callers_;
    // Tests will expect a DTMF mode; default is SIP INFO
    char dtmf_mode_[10];
    // Custom definition for 4-party layout. 
    // Experiment to make sure new overlays work. May be obsolete if JT
    // can adapt standard layouts internally
    
	static const char *custom2PartyLayout() { return "1=0,25,50;2=50,25,50"; }

	static const char *custom4PartyLayout() { return "1=0,0,50;2=0,50,50;3=50,0,50;4=50,50,50"; }

    static const char *custom6PartyLayout() { return "1=0,0,66.6;2=66.6,0,33.3;3=66.6,33.3,33.3;4=66.6,66.6,33.3;5=33.3,66.6,33.3;6=0,66.6,33.3";}

	static const char *custom9PartyLayout() { return "1=0,0,33.3;2=0,33.3,33.3;3=0,66.6,33.3;4=33.3,0,33.3;5=33.3,33.3,33.3;6=33.3,66.6,33.3;7=66.6,0,33.3;8=66.6,33.3,33.3;9=66.6,66.6,33.3";}

    // Overlay Definitions

    const std::string showCallerIdOverlay (int regionId, const char * callerName)
    {
        std::stringstream strstr;
        strstr <<    "region=" << regionId << ",overlay_id=caller_overlay,left=30%,top=90%,hsize=40%,vsize=10%,priority=0.6,overlay_duration=lifeOfContent,overlay_bgopacity=0%,textstyle_id=textStyle2,fontfamily=TimesNewRoman,fontweight=bold,fontcolor=firebrick,textstyle_bgcolor=gray,textalignment=center,content_id=r2-1,p_id=conferee_name,p_style=textStyle2,text=" << callerName;
		return strstr.str();
	}

    const std::string showVideoLabelOverlay (int regionId)
    {
        std::stringstream strstr;
        strstr <<    "region=" << regionId << ",overlay_id=video_overlay,left=30%,top=90%,hsize=40%,vsize=10%,priority=0.1,overlay_duration=lifeOfContent,overlay_bgopacity=0%,textstyle_id=textStyle2,fontfamily=TimesNewRoman,fontweight=bold,fontcolor=firebrick,textstyle_bgcolor=gray,textalignment=center,content_id=r2-1,p_id=ivideo_caption,p_style=textStyle2,text=Recorded Video";
		return strstr.str();
	}

    const std::string deleteVideoLabelOverlay (int regionId)
    {
        std::stringstream strstr;
        strstr <<    "region=" << regionId << ",overlay_id=video_overlay,priority=0";
		return strstr.str();
	}


    const std::string deleteCallerIdOverlay (int regionId)
    {
        std::stringstream strstr;
        strstr <<    "region=" << regionId << ",overlay_id=caller_overlay,priority=0";
		return strstr.str();
	}

    const std::string showMicOnOverlay (int regionId)
    {
        std::stringstream strstr;
        strstr <<    "region=" << regionId << ",overlay_id=micon_overlay,left=0%,top=0%,hsize=10%,vsize=10%,priority=0.3,overlay_bgopacity=0%,imgstyle_id=imgStyle1,imgalignment=center,imgstyle_bgopacity=0%,content_id=body1,content_applymode=replace,img_id=image1,img_style=imgStyle1,img_type=png,img_uri=file:///var/lib/xms/media/en-US/restconfdemo/red_recording_dot.png";
		return strstr.str();
	}

    const std::string deleteMicOnOverlay (int regionId)
    {
        std::stringstream strstr;
        strstr <<    "region=" << regionId << ",overlay_id=micon_overlay,priority=0";
		return strstr.str();
	}

    const std::string showMicMuteOverlay (int regionId)
    {
        std::stringstream strstr;
        strstr <<    "region=" << regionId << ",overlay_id=micmute_overlay,left=0%,top=0%,hsize=15%,vsize=105,priority=0.2,overlay_bgopacity=0%,imgstyle_id=imgStyle1,imgalignment=center,imgstyle_bgopacity=0%,content_id=body1,content_applymode=replace,img_id=image1,img_style=imgStyle1,img_type=png,img_uri=file:///var/lib/xms/media/en-US/restconfdemo/mic_disabled.png";
		return strstr.str();
	}

    const std::string deleteMicMuteOverlay (int regionId)
    {
        std::stringstream strstr;
        strstr <<    "region=" << regionId << ",overlay_id=micmute_overlay,priority=0";
		return strstr.str();
	}

    const std::string showBagheadOverlay (int regionId)
    {
        std::stringstream strstr;
        strstr <<    "region=" << regionId << ",overlay_id=baghead_overlay,left=0%,top=0%,hsize=100%,vsize=100%,priority=0.5,overlay_bgopacity=0%,imgstyle_id=imgStyle1,imgalignment=center,imgstyle_bgopacity=0%,content_id=body1,content_applymode=replace,img_id=image1,img_style=imgStyle1,img_type=png,img_uri=file:///var/lib/xms/media/en-US/restconfdemo/baghead.png";
		return strstr.str();
	}

    const std::string deleteBagheadOverlay (int regionId)
    {
        std::stringstream strstr;
        strstr <<    "region=" << regionId << ",overlay_id=baghead_overlay,priority=0";
		return strstr.str();
	}

    const std::string showStockTickerOverlay (int regionId)
    {
        std::stringstream strstr;
        strstr <<    "region=" << regionId << ",overlay_id=stocktick,left=20%,top=50%,hsize=60%,vsize=50%,priority=0.5,overlay_bgcolor=CornflowerBlue,imgstyle_id=imgStyle1,imgalignment=center,imgstyle_bgopacity=0%,content_id=body1,content_applymode=replace,img_id=image1,img_style=imgStyle1,img_type=png,img_uri=file:///var/lib/xms/media/en-US/restconfdemo/stock_market.png,img_duration=0;region=" << regionId << ",overlay_id=stocktickText,left=20%,top=95%,hsize=60%,vsize=5%,priority=0.4,overlay_bgcolor=gray,textstyle_id=textStyle2,fontfamily=Arial,fontstyle=normal,fontweight=bold,fonteffects=none,fontsize=100%,fontcolor=firebrick,fontdirection=lr,textstyle_bgcolor=gray,textalignment=center,wrap=nowrap,content_id=body1,content_applymode=replace,scroll_mode=scrollContinuous,direction=rl,padding=0,speed=8,p_id=textstring1,p_style=textStyle2,p_duration=30s,encoding=UTF8,text=Symbol - AMD   Name - Advanced Micro Devices   Stock Exchange - NYSE   Opening Price - 4.1501   Asking Price - N/A   EBITDA - -90.0M  Symbol - BA   Name - Boeing Company   Stock   Days Low - 139.75   52 Week High - 142.80   52 Week Low - 73.00  Earnings per Share - 80.88   Asking Price - N/A   Volume - 454977   Days High - 81.8525  Days Low - 80    Symbol - CAB   Name - Cabela's Inc Clas   Stock Exchange - NYSE   Opening Price - 69.66   Asking Low - 47.65  Earnings per Share - 2.959   EBITDA - 437.3M  Symbol - DOW   Name - Dow 5   Days High - 43.47  Days Low - 43.00   52 Week High - 44.99   52 Week Low - 29.81    Symbol - IBM   Name - Internaltion Business Machines   Opening Price - 1156.85   Asking Price - N/A   Volume - 2766140   Days High - 116   EBITDA - 17.599B";
		return strstr.str();
	}

    const std::string deleteStockTickerOverlay (int regionId)
    {
        std::stringstream strstr;
        strstr <<    "region=" << regionId << ",overlay_id=stocktick,priority=0;region=" << regionId << ",overlay_id=stocktickText,priority=0";

		return strstr.str();
	}

    const std::string showSlideOverlay (int regionId,int  slideId)
    {
        std::stringstream strstr;
        strstr <<    "region=" << regionId << ",overlay_id=slideshow_overlay,left=0%,top=0%,hsize=66.6%,vsize=66.6%,priority=0.4,hbwidth=2%,vbwidth=2%,bcolor=firebrick,overlay_duration=lifeOfContent,imgstyle_id=imgStyle1,imgalignment=center,imgstyle_applymode=resizeToFit,imgsize=98%,img_duration=5s,content_id=slide" << slideId << ",content_applymode=replace,img_id=image1,img_style=imgStyle1,img_type=png,img_uri=file:///var/lib/xms/media/en-US/restconfdemo/slide" << slideId << ".png";
		return strstr.str();
	}

    const std::string deleteSlideOverlay (int regionId)
    {
        std::stringstream strstr;
        strstr <<    "region=" << regionId << ",overlay_id=slideshow_overlay,priority=0";
		return strstr.str();
	}
    // Single conference in the demo
    bool scrolling_overlay_;
    bool slide_show_;
    std::string conf_id_;
    char layout_;
    const char *get_next_layout ();
    int file_exists (const char *filename);
    bool is_very_first_call_;
    // 9 possible conference tiles, either used or not
    bool region_use_[9];
    bool region_processed_[9];
    bool recordInProgress_;
    bool captionsOn_;
};



#endif // _CONFERENCE720P_H

/* vim:ts=4:set nu:
 * EOF
 */

