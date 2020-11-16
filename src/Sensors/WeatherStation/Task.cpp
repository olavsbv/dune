//***************************************************************************
// Copyright 2007-2020 Universidade do Porto - Faculdade de Engenharia      *
// Laboratório de Sistemas e Tecnologia Subaquática (LSTS)                  *
//***************************************************************************
// This file is part of DUNE: Unified Navigation Environment.               *
//                                                                          *
// Commercial Licence Usage                                                 *
// Licencees holding valid commercial DUNE licences may use this file in    *
// accordance with the commercial licence agreement provided with the       *
// Software or, alternatively, in accordance with the terms contained in a  *
// written agreement between you and Faculdade de Engenharia da             *
// Universidade do Porto. For licensing terms, conditions, and further      *
// information contact lsts@fe.up.pt.                                       *
//                                                                          *
// Modified European Union Public Licence - EUPL v.1.1 Usage                *
// Alternatively, this file may be used under the terms of the Modified     *
// EUPL, Version 1.1 only (the "Licence"), appearing in the file LICENCE.md *
// included in the packaging of this file. You may not use this work        *
// except in compliance with the Licence. Unless required by applicable     *
// law or agreed to in writing, software distributed under the Licence is   *
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF     *
// ANY KIND, either express or implied. See the Licence for the specific    *
// language governing permissions and limitations at                        *
// https://github.com/LSTS/dune/blob/master/LICENCE.md and                  *
// http://ec.europa.eu/idabc/eupl.html.                                     *
//***************************************************************************
// Author: Olav Vassbotn                                                    *
//***************************************************************************

// DUNE headers.
#include <DUNE/DUNE.hpp>
#include <random>
#include <chrono>

namespace Sensors
{
  //! This is the implementation of a weather station in dune
  //! It is intended to simulate windspeed, temperature and humidity
  //! All simulated values will be from additive white noise,
  //! The variation of simulated values can also be selected
  //!
  //!
  //! @author Olav Vassbotn
  namespace WeatherStation
  {
    using DUNE_NAMESPACES;

    //! Power states

    enum PowerState
    {
	//! Power off
	PWR_OFF = 0,
	//! Power on
	PWR_ON  = 1
    };

    struct Arguments
    {
      //! Seed
      int seed;

      //! State of sensor
      int state;

      //! Message interval
      float time_interval;

      //! Initial temerature
      float temp_sim;

      //! Bool to determine if the temperature 
      //! can deviate much from the initial point or not
      bool temp_stay_on;

      //! Initial humidity 
      float humidity_sim;

      //! Bool to determine if the humidity 
      //! can deviate much from the initial point or not
      bool humidity_stay_on;

      //! Initial windspeed
      float windspeed_sim;

      //! Initial wind direction
      float winddirection_sim;

      //! Bool to determine if the windspeed 
      //! can deviate much from the initial point or not
      bool wind_stay_on;

      //! Temperature standard deviation
      float temp_deviation;

      //! Humidity standard deviation
      float humidity_deviation;

      //! Windspeed standard deviation
      float windspeed_deviation;

      //! Winddirection standard deviation
      float winddirection_deviation;

      //! Measurement standard deviation
      float measurement_deviation;
    };

    struct Task: public DUNE::Tasks::Task
    {
      //! Time at last message
      unsigned long time = std::chrono::system_clock::now().time_since_epoch() / 
    		           std::chrono::milliseconds(1); 

      //! Simulated temperature
      float temp_sim;

      //! Timeout temp_msg
      int temp_tm = 0;

      //! Conflicting temparature
      bool temp_conflicting = false;

      //! Measured temperature
      float m_temp;

      //! Simulated humidity
      float humidity_sim;

      //! Timeout humidity_msg
      int humidity_tm = 0;

      //! Conflicting humidity
      bool humidity_conflicting = false;

      //! Measured humidity
      float m_humidity;

      //! Simulated windspeed
      float windspeed_sim;

      //! Initial wind direction
      float winddirection_sim;

      //! Conflicting windspeed
      bool wind_conflicting = false;

      //! Measured windspeed
      float m_windspeed;

      //! Timeout wind_msg
      int wind_tm = 0;

      //! Temperature message
      IMC::Temperature temperature_msg;

      //! Humidity messgage
      IMC::RelativeHumidity humidity_msg;
      
      //! Windspeed messgage
      IMC::WindSpeed windspeed_mgs;

      Arguments args;

      //std::default_random_engine generator;
      unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
      std::default_random_engine generator;

      Task(const std::string& name, Tasks::Context& ctx):
        DUNE::Tasks::Task(name, ctx)
      {
	param("Message interval", args.time_interval)
	.defaultValue("1000")
	.description("Sensor message interval in milliseconds");

	param("Initial temperature value", args.temp_sim)
	.defaultValue("20.0")
	.description("This is the initial temperature value");

	param("Stay about temperature initual value", args.temp_stay_on)
	.defaultValue("false")
	.description("If true the temperature will not deviate from the initial value too much");

	param("Initial humidity value", args.humidity_sim)
	.defaultValue("10.0")
	.minimumValue("0")
	.maximumValue("100")
	.description("This is the initial humidity value");

	param("Stay about humudity initual value", args.humidity_stay_on)
	.defaultValue("false")
	.description("If true the humidity will not deviate from the initial value too much");

	param("Initial windspeed value", args.windspeed_sim)
	.defaultValue("0.0")
	.description("This is the initial windspeed value");

	param("Initial windsdirection value", args.windspeed_sim)
	.defaultValue("0.0")
	.description("This is the initial windspeed value");

	param("Stay about wind's initual value", args.wind_stay_on)
	.defaultValue("false")
	.description("If true the wind speed and direction will not deviate from the initial value too much");

	param("Temperature standard devaition", args.temp_deviation)
	.defaultValue("2.0")
	.description("The temperature standard deviation in the simulation of temperature");

	param("Humidity standard devaition", args.humidity_deviation)
	.defaultValue("2.0")
	.description("The humidity standard deviation in the simulation of humidity");

	param("Windspeed standard devaition", args.windspeed_deviation)
	.defaultValue("2.0")
	.description("The windspeed standard deviation in the simulation of windspeed");

	param("Wind direction standard devaition", args.winddirection_deviation)
	.defaultValue("2.0")
	.description("The wind direction standard deviation in the simulation of wind direction");

	param("Measurement standard devaition", args.measurement_deviation)
	.defaultValue("0.5")
	.description("The measurement standard deviation in the simulation of measurement");

	param("Power mode", args.state)
	.defaultValue("1")
	.minimumValue("0")
	.maximumValue("1")
	.description("Power mode of sensor: 0 - Off, 1 - On");

	bind<IMC::Temperature>(this);
	bind<IMC::RelativeHumidity>(this);
	bind<IMC::WindSpeed>(this);
      }

      void
      consume(const IMC::Temperature* msg)
      {
	if (temp_conflicting && args.temp_sim != 20)
	{
	  war("There are conflicting Temperature messages, the sensor will stop simulating temperature");
	  temp_conflicting = true;
	}

	temp_tm = 0;
	if (msg->value != 0)
	  temp_sim = msg->value;
      }

      void
      consume(const IMC::WindSpeed* msg)
      {
	wind_tm = 0;

	if (wind_conflicting && args.windspeed_sim != 10)
	{
	  war("There are conflicting windspeed messages, the sensor will stop simulating windspeed");
	  wind_conflicting = true;
	}

	if (msg->speed != 0)
	  windspeed_sim = msg->speed;
	if (msg->direction != 0)
	  winddirection_sim = msg->direction;
      }

      void
      consume(const IMC::RelativeHumidity* msg)
      {
	if (humidity_conflicting && args.humidity_sim != 10)
	{
	  war("There are conflicting humidty messages, the sensor will stop simulating humidity");
	  wind_conflicting = true;
	}

	humidity_tm = 0;
	if (msg->value != 0)
	  humidity_sim = msg->value;
      }

      //! Update internal state with new parameter values.
      void
      onUpdateParameters(void)
      {
	if (paramChanged(args.temp_sim))
	  temp_sim = args.temp_sim;

	if (paramChanged(args.humidity_sim))
	  humidity_sim = args.humidity_sim;

	if (paramChanged(args.windspeed_sim))
	  windspeed_sim = args.windspeed_sim;
      }

      //! Reserve entity identifiers.
      void
      onEntityReservation(void)
      {
      }

      //! Resolve entity names.
      void
      onEntityResolution(void)
      {
      }

      //! Acquire resources.
      void
      onResourceAcquisition(void)
      {
      }

      //! Initialize resources.
      void
      onResourceInitialization(void)
      {
      }

      //! Release resources.
      void
      onResourceRelease(void)
      {
	//Memory::clear(humidity_msg);
      }

      //! Main loop.
      void
      onMain(void)
      {
        while (!stopping())
        {
	  //! Update simulated values
	  std::normal_distribution<float> measure_noise_distribution{0, args.measurement_deviation};
	  
	  //! Temperature
	  if(temp_tm >= 5)
	  {
	    if (args.temp_stay_on)
            {
      	      std::normal_distribution<float> temp_t_distribution{(args.temp_sim - temp_sim)/10, args.temp_deviation};
	      temp_sim += temp_t_distribution(generator);
	    }
	    else
            {
      	      std::normal_distribution<float> temp_t_distribution{0,args.temp_deviation};
	      temp_sim += temp_t_distribution(generator);
	    }
	  }
	  else temp_tm++;
	  temperature_msg.value = temp_sim + measure_noise_distribution(generator);
	
	  //! Humidity
	  if(humidity_tm >= 5)
	  {
	    if (args.humidity_stay_on)
            {
      	      std::normal_distribution<float> humidity_t_distribution{(args.humidity_sim - humidity_sim)/10,args.humidity_deviation};
	      humidity_sim += humidity_t_distribution(generator);
	    }
	    else
            {
      	      std::normal_distribution<float> humidity_t_distribution{0,args.humidity_deviation};
	      humidity_sim += humidity_t_distribution(generator);
	    }

	    if (humidity_sim < 0)
	      humidity_sim = 0;
	    else if (humidity_sim > 100)
	      humidity_sim = 100;
	  }
	  else humidity_tm++;
	  humidity_msg.value = humidity_sim + measure_noise_distribution(generator);

	  //! Wind 
	  if(wind_tm >= 5)
	  {
	    if (args.wind_stay_on)
            {
      	      std::normal_distribution<float> windspeed_t_distribution{(args.windspeed_sim - windspeed_sim)/10,args.windspeed_deviation};
	      windspeed_sim += windspeed_t_distribution(generator);

	      float diff_angle = args.winddirection_sim - winddirection_sim;
	      if (diff_angle > 180)
		diff_angle -= 360;
	      else if (diff_angle < -180)
		diff_angle += 360;

      	      std::normal_distribution<float> winddirection_t_distribution{(diff_angle)/10, args.winddirection_deviation};
	      winddirection_sim += winddirection_t_distribution(generator);
	    }
	    else
            {
      	      std::normal_distribution<float> windspeed_t_distribution{0, args.windspeed_deviation};
	      windspeed_sim += windspeed_t_distribution(generator);

      	      std::normal_distribution<float> winddirection_t_distribution{0, args.winddirection_deviation};
	      winddirection_sim += winddirection_t_distribution(generator);
	    }

	    if (windspeed_sim < 0)
	      windspeed_sim = 0;
	    if (winddirection_sim < 0)
	      winddirection_sim += 360;
	    else if (winddirection_sim > 360)
	      winddirection_sim -= 360;

	  }
	  else wind_tm++;

	  windspeed_mgs.speed = windspeed_sim + measure_noise_distribution(generator);
	  windspeed_mgs.direction = winddirection_sim + measure_noise_distribution(generator);

          unsigned long time_now = std::chrono::system_clock::now().time_since_epoch() / 
    		                   std::chrono::milliseconds(1); 

	  if (args.state == 1 && time_now - time > args.time_interval)
	  {
	    dispatch(temperature_msg);
	    dispatch(humidity_msg);
	    dispatch(windspeed_mgs);
      	    time = std::chrono::system_clock::now().time_since_epoch() / 
    		           std::chrono::milliseconds(1); 
	  }

	  waitForMessages(args.time_interval / 1000 / 2); // This is purely to keep CPU at minimum
        }
      }
    };
  }
}

DUNE_TASK
