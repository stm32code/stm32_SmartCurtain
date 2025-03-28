package com.example.smartcurtain

import android.content.Intent
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.view.Gravity
import android.view.Menu
import android.view.MenuItem
import android.view.View
import android.widget.ArrayAdapter
import android.widget.SeekBar
import com.blankj.utilcode.util.LogUtils
import com.example.SmartCurtain_2.R
import com.example.SmartCurtain_2.databinding.ActivityMainBinding
import com.example.smartcurtain.bean.DataDTO
import com.example.smartcurtain.bean.Receive
import com.example.smartcurtain.bean.Send
import com.example.smartcurtain.utils.Common
import com.example.smartcurtain.utils.Common.PushTopic
import com.example.smartcurtain.utils.MToast
import com.example.smartcurtain.utils.TimeCycle
import com.google.gson.Gson
import com.gyf.immersionbar.ImmersionBar
import com.itfitness.mqttlibrary.MQTTHelper
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken
import org.eclipse.paho.client.mqttv3.MqttCallback
import org.eclipse.paho.client.mqttv3.MqttMessage
import java.util.Objects

class MainActivity : AppCompatActivity() {
    private lateinit var binding: ActivityMainBinding
    private var isDebugView = false //是否显示debug界面
    private var arrayList = ArrayList<String>()// debug消息数据
    private var adapter: ArrayAdapter<String>? = null
    private var onlineFlag = false //是否在线标识
    private var TAG = this.toString()
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        mqttServer()
        initView()
        isOnline()
    }


    /**
     * @brief 界面的初始化
     */
    private fun initView() {
        setSupportActionBar(findViewById(R.id.toolbar))
        binding.toolbarLayout.title = title
        ImmersionBar.with(this).init()
        debugView()
        eventManager()
    }

    /**
     * 事件处理
     */
    private fun eventManager() {
        binding.modeSwitch.setOnClickListener {
            sendMessage(1, if (binding.modeSwitch.isChecked) "0" else "1")
        }

        binding.waterSeekBar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(p0: SeekBar?, p1: Int, p2: Boolean) {
                if (Common.mqttHelper?.connected == true) {
                    binding.waterSeekText.text = p1.toString()
                } else {
                    MToast.mToast(this@MainActivity, "请先建立连接")
                }
            }

            override fun onStartTrackingTouch(p0: SeekBar?) {

            }

            override fun onStopTrackingTouch(p0: SeekBar?) {
                if (p0 != null) {
                    if (Common.mqttHelper?.connected == true) {
                        val p1 = p0.progress.toString()
                        binding.waterSeekText.text = p1
                        sendMessage(2, p1)
                    } else {
                        MToast.mToast(this@MainActivity, "请先建立连接")
                    }
                }
            }

        })
    }

    /**
     * 接收消息处理
     */
    private fun analysisOfData(data: Receive?) {
          }

    /**
     * @brief 再次封装MQTT发送
     * @param message 需要发送的消息
     */
    private fun sendMessage(cmd: Int, message: String) {
        if (Common.mqttHelper != null && Common.mqttHelper!!.subscription) {
            var str = ""
            when (cmd) {
                1 -> {
                    str = Gson().toJson(
                        Send(
                            cmd, DataDTO(
                                flage = message.toInt()
                            )
                        )
                    )
                }

                2 -> {
                    str = Gson().toJson(
                        Send(
                            cmd, DataDTO(
                                window = message.toInt()
                            )
                        )
                    )
                }

                3 -> {
                    str = Gson().toJson(
                        Send(
                            cmd, DataDTO(
                                time = message
                            )
                        )
                    )
                }

            }
            Thread {
                Common.mqttHelper!!.publish(
                    PushTopic, str, 1
                )
            }.start()

            debugViewData(1, str)
        }
    }

    private fun mqttServer() {
        Common.mqttHelper = MQTTHelper(
            this, Common.Sever, Common.DriveID, Common.DriveName, Common.DrivePassword, true, 30, 60
        )
        Common.mqttHelper!!.connect(Common.ReceiveTopic, 1, true, object : MqttCallback {
            override fun connectionLost(cause: Throwable?) {
                MToast.mToast(this@MainActivity, "MQTT连接断开")
            }

            override fun messageArrived(topic: String?, message: MqttMessage?) {
                //收到消息
                val data = Gson().fromJson(message.toString(), Receive::class.java)
                LogUtils.eTag("接收到消息", message?.payload?.let { String(it) })
                onlineFlag = true
                binding.online.text = "在线"
                debugViewData(2, message?.payload?.let { String(it) }!!)
                println(data)
                analysisOfData(data)
            }

            override fun deliveryComplete(token: IMqttDeliveryToken?) {

            }
        })

    }

    /***
     * 判断硬件是否在线
     */
    private fun isOnline() {
        Thread {
            var i = 0
            while (true) {//开启轮询，每18s重置标识
                if (i > 3) {
                    i = 0
                    runOnUiThread {
                        binding.online.text = if (onlineFlag) "在线" else "离线"
                    }
                    onlineFlag = false
                }
                i++
                Thread.sleep(10000)
//                Thread.sleep(2500)
            }
        }.start()
    }


    /**
     * @brief debug界面的初始化
     */
    private fun debugView() {
        adapter = ArrayAdapter(this, android.R.layout.simple_list_item_1, arrayList)
        binding.debugView.adapter = adapter
    }

    /**
     * @brief debug界面数据添加
     * @param str 如果为 1 添加发送数据到界面   为 2 添加接受消息到界面
     */
    private fun debugViewData(str: Int, data: String) {
        if (arrayList.size >= 255) arrayList.clear()
        runOnUiThread {
            when (str) {
                1 -> { //发送的消息
                    arrayList.add("目标主题:${Common.ReceiveTopic} \n时间:${TimeCycle.getDateTime()}\n发送消息:$data ")
                }

                2 -> {
                    arrayList.add("来自主题:${Common.ReceiveTopic} \n时间:${TimeCycle.getDateTime()}\n接到消息:$data ")
                }
            }
            // 在添加新数据之后调用以下方法，滚动到列表底部
            binding.debugView.post {
                binding.debugView.setSelection(adapter?.count?.minus(1)!!)
            }
            adapter?.notifyDataSetChanged()
        }

    }


    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.menu_scrolling, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            R.id.setDebugView -> { // set debug view is enabled or disabled
                isDebugView = !isDebugView
                binding.debugView.visibility = if (isDebugView) View.VISIBLE else View.GONE
                true
            }

            else -> super.onOptionsItemSelected(item)
        }
    }

}